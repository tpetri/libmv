/****************************************************************************
**
** Copyright (c) 2011 libmv authors.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
**
****************************************************************************/

#include "klt.h"

#include "libmv/numeric/numeric.h"
#include "libmv/image/image.h"
#include "libmv/image/convolve.h"
#include "libmv/image/sample.h"

namespace libmv {

Tracker::Tracker(int half_pattern_size, int search_size, int num_levels) :
  half_pattern_size(half_pattern_size),
  search_size(search_size),
  num_levels(num_levels),
  max_iterations(16),
  tolerance(0.2),
  min_determinant(1e-6),
  min_update_squared_distance(1e-6),
  sigma(0.9),
  lambda(0.05) {}

static void MakePyramid(const FloatImage &image, int num_levels,
                        std::vector<FloatImage> *pyramid) {
  pyramid->resize(num_levels);
  (*pyramid)[0] = image;
  for (int i = 1; i < num_levels; ++i) {
    DownsampleChannelsBy2((*pyramid)[i - 1], &(*pyramid)[i]);
  }
}

bool Tracker::Track(const FloatImage &image1,
                    const FloatImage &image2,
                    float  x1, float  y1,
                    float *x2, float *y2) const {
  // Create all the levels of the pyramid, since tracking has to happen from
  // the coarsest to finest levels, which means holding on to all levels of the
  // pyramid at once.
  std::vector<FloatImage> pyramid1(num_levels);
  std::vector<FloatImage> pyramid2(num_levels);
  MakePyramid(image1, num_levels, &pyramid1);
  MakePyramid(image2, num_levels, &pyramid2);

  // Track forward, getting x2 and y2.
  if (!TrackPyramid(pyramid1, pyramid2, x1, y1, x2, y2)) {
    return false;
  }
  // Now track x2 and y2 backward, to get xx1 and yy1 which, if the track is
  // good, should match x1 and y1 (but may not if the track is bad).
  float xx1 = *x2, yy1 = *x2;
  if (!TrackPyramid(pyramid2, pyramid1, *x2, *y2, &xx1, &yy1)) {
    return false;
  }
  float dx = xx1 - x1;
  float dy = yy1 - y1;
  return sqrt(dx * dx + dy * dy) < tolerance;
}

bool Tracker::TrackPyramid(std::vector<FloatImage> pyramid1,
                           std::vector<FloatImage> pyramid2,
                    float  x1, float  y1,
                    float *x2, float *y2) const {
  // Shrink the guessed x and y location to match the coarsest level + 1 (which
  // when gets corrected in the loop).
  *x2 /= pow(2., num_levels);
  *y2 /= pow(2., num_levels);

  for (int i = num_levels - 1; i >= 0; --i) {
    // Position in the first image at pyramid level i.
    float xx = x1 / pow(2., i);
    float yy = y1 / pow(2., i);

    // Guess the new tracked position is where the last level tracked to.
    *x2 *= 2;
    *y2 *= 2;

    // Track the point on this level with the base tracker.
    bool succeeded = TrackImage(pyramid1[i], pyramid2[i], xx, yy, x2, y2);

    if (i == 0 && !succeeded) {
      // Only fail on the highest-resolution level, because a failure on a
      // coarse level does not mean failure at a lower level (consider
      // out-of-bounds conditions).
      return false;
    }
  }
  return true;
}

// Computes U and e from the Ud = e equation (number 14) from the paper.
static void ComputeTrackingEquation(const Array3Df &image_and_gradient1,
                                    const Array3Df &image_and_gradient2,
                                    float x1, float y1,
                                    float x2, float y2,
                                    int half_width,
                                    float lambda,
                                    Mat2f *U,
                                    Vec2f *e) {
  Mat2f A, B, C, D;
  A = B = C = D  = Mat2f::Zero();

  Vec2f R, S, V, W;
  R = S = V = W = Vec2f::Zero();

  for (int r = -half_width; r <= half_width; ++r) {
    for (int c = -half_width; c <= half_width; ++c) {
      float xx1 = x1 + c;
      float yy1 = y1 + r;
      float xx2 = x2 + c;
      float yy2 = y2 + r;

      float I = SampleLinear(image_and_gradient1, yy1, xx1, 0);
      float J = SampleLinear(image_and_gradient2, yy2, xx2, 0);

      Vec2f gI, gJ;
      gI << SampleLinear(image_and_gradient1, yy1, xx1, 1),
          SampleLinear(image_and_gradient1, yy1, xx1, 2);
      gJ << SampleLinear(image_and_gradient2, yy2, xx2, 1),
          SampleLinear(image_and_gradient2, yy2, xx2, 2);

      // Equation 15 from the paper.
      A += gI * gI.transpose();
      B += gI * gJ.transpose();
      C += gJ * gJ.transpose();
      R += I * gI;
      S += J * gI;
      V += I * gJ;
      W += J * gJ;
    }
  }

  // In the paper they show a D matrix, but it is just B transpose, so use that
  // instead of explicitly computing D.
  Mat2f Di = B.transpose().inverse();

  // Equation 14 from the paper.
  *U = A*Di*C + lambda*Di*C - 0.5*B;
  *e = (A + lambda*Mat2f::Identity())*Di*(V - W) + 0.5*(S - R);
}

static bool SolveTrackingEquation(const Mat2f &U,
                                  const Vec2f &e,
                                  float min_determinant,
                                  Vec2f *d) {
  float det = U.determinant();
  if (det < min_determinant) {
    d->setZero();
    return false;
  }
  *d = U.lu().solve(e);
  return true;
}

bool Tracker::TrackImage(const FloatImage &image1,
                         const FloatImage &image2,
                         float  x1, float  y1,
                         float *x2, float *y2) const {
  Array3Df image_and_gradient1;
  Array3Df image_and_gradient2;
  BlurredImageAndDerivativesChannels(image1, sigma, &image_and_gradient1);
  BlurredImageAndDerivativesChannels(image2, sigma, &image_and_gradient2);

  int i;
  Vec2f d = Vec2f::Zero();
  for (i = 0; i < max_iterations; ++i) {
    // Compute gradient matrix and error vector.
    Mat2f U;
    Vec2f e;
    ComputeTrackingEquation(image_and_gradient1,
                            image_and_gradient2,
                            x1, y1,
                            *x2, *y2,
                            half_pattern_size,
                            lambda,
                            &U, &e);

    // Solve the linear system for the best update to x2 and y2.
    if (!SolveTrackingEquation(U, e, min_determinant, &d)) {
      // The determinant, which indicates the trackiness of the point, is too
      // small, so fail out.
      return false;
    }

    // Update the position with the solved displacement.
    *x2 += d[0];
    *y2 += d[1];

    // If the update is small, then we probably found the target.
    if (d.squaredNorm() < min_update_squared_distance) {
      return true;
    }
  }
  // Getting here means we hit max iterations, so tracking failed.
  return false;
}

}  // namespace libmv
