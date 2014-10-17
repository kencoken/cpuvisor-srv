cmake_minimum_required(VERSION 2.6)

project(cpuvisorsrv)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/aux/cmake/")

set (CMAKE_SKIP_BUILD_RPATH FALSE)
set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set (CMAKE_INSTALL_RPATH "lib")
set (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} "\$ORIGIN")

# CONFIG OPTIONS
# -------------------------------------

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release
RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

find_package(Git)
if (GIT_FOUND)
  include(GetGitRevisionDescription)
  get_git_head_revision(GIT_REFSPEC GIT_SHA1)
  add_definitions("-D GIT_SHA1=${GIT_SHA1}")
endif (GIT_FOUND)

# FIND LIBRARIES
# -------------------------------------
#set(Boost_NO_SYSTEM_PATHS TRUE)
find_package(Boost 1.53.0 REQUIRED COMPONENTS thread system filesystem)
find_package(Liblinear REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Caffe REQUIRED)
find_package(GLOG REQUIRED)
find_package(GFLAGS REQUIRED)
find_package(Protobuf REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(cppnetlib 0.11.0 REQUIRED)

# SET INCLUDE DIRECTORIES
# -------------------------------------
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Liblinear_INCLUDE_DIRS})
include_directories(${Caffe_INCLUDE_DIRS})
include_directories(${GLOG_INCLUDE_DIRS})
include_directories(${GFLAGS_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${ZeroMQ_INCLUDE_DIRS})
include_directories(${CPPNETLIB_INCLUDE_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/src")

# SET FLAGS
# -------------------------------------
# (not editable via ccmake, as are overridden here -
# this would also be the case if the variables were non-cached
# but let's cache them anyway so they at least appear in ccmake)
set(CMAKE_CXX_FLAGS "-g -Wall -march=native -DCPU_ONLY" # flags added to all configs - and also when no config specified (default)
  CACHE STRING "Edit in CMakeLists.txt" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG ""#"-D_GLIBCXX_DEBUG"
  CACHE STRING "Edit in CMakeLists.txt" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG"
  CACHE STRING "Edit in CMakeLists.txt" FORCE)
# other configuration types
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2"
  CACHE STRING "Edit in CMakeLists.txt" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG"
  CACHE STRING "Edit in CMakeLists.txt" FORCE)

# add BLAS headers directory if OSX
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/System/Library/Frameworks/vecLib.framework/Versions/A/Headers/"
    CACHE STRING "Edit in CMakeLists.txt" FORCE)
endif ()

# ADD SUBDIRECTORIES TO START BUILD
# -------------------------------------
add_subdirectory(src/proto proto)
add_subdirectory(src bin)
#add_subdirectory(src/test bin/test)
