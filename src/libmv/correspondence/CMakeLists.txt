FIND_PACKAGE( OpenCV REQUIRED )

# define the source files
SET(CORRESPONDENCE_SRC 
                       matches.cc 
                       feature_matching.cc
#                      feature_matching_FLANN.cc
                       tracker.cc
                       robust_tracker.cc
                       planar_tracker.cc
                       nRobustViewMatching.cc
)

# define the header files (make the headers appear in IDEs.)
FILE(GLOB CORRESPONDENCE_HDRS *.h)

ADD_LIBRARY(correspondence ${CORRESPONDENCE_SRC} ${CORRESPONDENCE_HDRS})
TARGET_LINK_LIBRARIES(correspondence multiview image flann)

# make the name of debug libraries end in _d.
SET_TARGET_PROPERTIES(correspondence PROPERTIES DEBUG_POSTFIX "_d")

TARGET_LINK_LIBRARIES(correspondence multiview flann image numeric)

# installation rules for the library
LIBMV_INSTALL_LIB(correspondence)
            
LIBMV_TEST(bipartite_graph "")
LIBMV_TEST(kdtree "")
LIBMV_TEST(matches "correspondence;image;numeric")
LIBMV_TEST(Array_Matcher "correspondence;numeric;flann")
# LIBMV_TEST(tracker "correspondence;reconstruction;numeric;flann")
