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

#ifndef UI_TRACKER_TRACKER_H_
#define UI_TRACKER_TRACKER_H_

#include <QMap>
#include <QGLWidget>
#include "ui/tracker/gl.h"

//TODO: Qt Tracker should be independent from libmv to be able to use new lens distortion API
#include "libmv/simple_pipeline/camera_intrinsics.h"
#include "libmv/tracking/sad.h"

// TODO(MatthiasF): custom pattern/search size
static const int kPatternSize = 32;
static const int kSearchSize = 64;

class Scene;

typedef unsigned char ubyte;

class Tracker : public QGLWidget {
  Q_OBJECT
 public:
  Tracker(libmv::CameraIntrinsics* intrinsics);

  void Load(QString path);
  void Save(QString path);
  void SetImage(int id, QImage image);
  void SetUndistort(bool undistort);
  void SetOverlay(Scene* scene);
  void Track(int previous, int next, QImage old, QImage search);
  void Render(int x, int y, int w, int h, int image=-1, int track=-1);

 public slots:
  void select(QVector<int>);
  void deleteSelectedMarkers();
  void deleteSelectedTracks();
  void upload();

 signals:
  void trackChanged(QVector<int> tracks);

 protected:
  void paintGL();
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

 private:
  void DrawMarker(const libmv::mat32& marker, QVector<vec2> *lines);

  libmv::CameraIntrinsics* intrinsics_;
  Scene* scene_;
  int last_frame;
  QVector< ubyte* > references;
  QVector< QVector<libmv::mat32> > tracks; //[track][image]

  bool undistort_;
  QImage image_;
  GLTexture texture_;
  mat4 transform_;
  GLBuffer markers_;
  int current_;
  QVector<int> selected_tracks_;
  vec2 last_position_;
  int active_track_;
  bool dragged_;
};

#endif
