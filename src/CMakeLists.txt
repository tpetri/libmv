# Copyright (c) 2007-2011 libmv authors.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

CMAKE_MINIMUM_REQUIRED(VERSION 2.2)

IF (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 OLD)
  cmake_policy(SET CMP0005 OLD)
ENDIF (COMMAND cmake_policy)

PROJECT(LIBMV C CXX)

#ENABLE_TESTING()

SET(CMAKE_MODULE_PATH
  "${LIBMV_SOURCE_DIR}/CMake"
  "${LIBMV_SOURCE_DIR}/CMake/modules")

INCLUDE(LibmvVersion)

SET(LIBMV_SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR})
SET(LIBMV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(LIBMV_BINARY_DIR  ${PROJECT_BINARY_DIR}/bin)
SET(LIBMV_LIBRARY_DIR ${PROJECT_BINARY_DIR}/lib)

SET(LIBMV_EXECUTABLE_OUTPUT_DIR bin)
SET(LIBMV_LIBRARY_OUTPUT_DIR    lib)
IF(NOT WIN32)
  SET(LIBMV_HEADERS_OUTPUT_DIR  include/libmv)
  SET(LIBMV_SHARE_OUTPUT_DIR    share/libmv)
ELSE(NOT WIN32)
  SET(LIBMV_HEADERS_OUTPUT_DIR  include)
  SET(LIBMV_SHARE_OUTPUT_DIR    ./)
ENDIF(NOT WIN32)
SET(LIBMV_TESTS_OUTPUT_DIR      ${LIBMV_BINARY_DIR}/tests)

IF(UNIX)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBMV_LIBRARY_DIR})
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBMV_LIBRARY_DIR})
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIBMV_BINARY_DIR})
ENDIF(UNIX)

IF(MSVC)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${LIBMV_LIBRARY_DIR})
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIBMV_LIBRARY_DIR})
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${LIBMV_LIBRARY_DIR})
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIBMV_LIBRARY_DIR})
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${LIBMV_BINARY_DIR})
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${LIBMV_BINARY_DIR})
ENDIF(MSVC)

# To build shared libs
OPTION(BUILD_SHARED_LIBS "Build dynamic shared libraries (.dll/.so)." ON)

INCLUDE(ConfigureBuild)

INCLUDE_DIRECTORIES(.)

INCLUDE_DIRECTORIES(SYSTEM
  third_party/daisy/include
  third_party/gtest
  third_party/gtest/include
  third_party/eigen
  third_party/eigen/unsupported
  third_party/ssba
  third_party/OpenExif/src
  third_party/OpenExif/src/OpenExifJpeg
)

IF (WIN32)
  INCLUDE_DIRECTORIES(
    third_party/msinttypes
    third_party/jpeg-7
    third_party/zlib
    third_party/pthreads-w32/include
    third_party/png)
ENDIF (WIN32)

IF (APPLE)
  INCLUDE_DIRECTORIES(
    third_party/png
    third_party/jpeg-7
    ${PROJECT_BINARY_DIR}/third_party/jpeg-7
  )
ENDIF (APPLE)


find_package (Cholmod)


MESSAGE("-- Check for Google Log")
  FIND_LIBRARY(GLOG_LIB NAMES glog PATHS /usr/local/lib /usr/local/lib64)
  IF (NOT EXISTS ${GLOG_LIB})
    MESSAGE(FATAL_ERROR
            "Can't find Google Log. Please specify: "
            "-DGLOG_LIB=...")
  ENDIF (NOT EXISTS ${GLOG_LIB})
  MESSAGE("-- Found Google Log library: ${GLOG_LIB}")

  FIND_PATH(GLOG_INCLUDE NAMES glog/logging.h PATHS /usr/local/include)
  IF (NOT EXISTS ${GLOG_INCLUDE})
    MESSAGE(FATAL_ERROR
            "Can't find Google Log. Please specify: "
            "-DGLOG_INCLUDE=...")
  ENDIF (NOT EXISTS ${GLOG_INCLUDE})
  MESSAGE("-- Found Google Log header in: ${GLOG_INCLUDE}")






INCLUDE(Documentation)
INCLUDE(Installation)
INCLUDE(Testing)

ADD_SUBDIRECTORY(libmv)
ADD_SUBDIRECTORY(third_party)

#IF (BUILD_TESTS)
#  ADD_SUBDIRECTORY(testing)
#ENDIF (BUILD_TESTS)

#OPTION(BUILD_GUI "Build Qt Tracker" ON)
#IF (BUILD_GUI)
# MESSAGE(STATUS "GUI enabled. Make sure you have Qt (and qt-devel for binary distributions)")
# ADD_SUBDIRECTORY(ui)
#ENDIF (BUILD_GUI)

#OPTION(BUILD_TOOLS "Build the command line tools." OFF)
#IF (BUILD_TOOLS)
#  ADD_SUBDIRECTORY(tools)
#ENDIF (BUILD_TOOLS)

INCLUDE(Packaging)

# opencv code
ADD_SUBDIRECTORY(libmv_opencv)
