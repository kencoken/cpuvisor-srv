# - Try to find Glog
#
# The following variables are optionally searched for defaults
#  GLOG_DIR:            Base directory where all GLOG components are found
#
# The following are set after configuration is done:
#  GLOG_FOUND
#  GLOG_INCLUDE_DIRS
#  GLOG_LIBRARIES

include(FindPackageHandleStandardArgs)

set(GLOG_DIR "" CACHE PATH "Folder contains Google glog")

if(WIN32)
    find_path(GLOG_INCLUDE_DIR glog/logging.h
        PATHS ${GLOG_DIR}/src/windows)
else(WIN32)
    find_path(GLOG_INCLUDE_DIR glog/logging.h
        PATHS ${GLOG_DIR})
endif(WIN32)

if(MSVC)
    find_library(GLOG_LIBRARY_RELEASE libglog_static
        PATHS ${GLOG_DIR}
        PATH_SUFFIXES Release)

    find_library(GLOG_LIBRARY_DEBUG libglog_static
        PATHS ${GLOG_DIR}
        PATH_SUFFIXES Debug)

    set(GLOG_LIBRARY optimized ${GLOG_LIBRARY_RELEASE} debug ${GLOG_LIBRARY_DEBUG})
else(MSVC)
    find_library(GLOG_LIBRARY glog
        PATHS ${GLOG_DIR}
        PATH_SUFFIXES
            lib
            lib64)
endif(MSVC)

find_package_handle_standard_args(GLOG DEFAULT_MSG
    GLOG_INCLUDE_DIR GLOG_LIBRARY)

if(GLOG_FOUND)
    set(GLOG_INCLUDE_DIRS ${GLOG_INCLUDE_DIR})
    set(GLOG_LIBRARIES ${GLOG_LIBRARY})
endif(GLOG_FOUND)

mark_as_advanced(
  GLOG_INCLUDE_DIR
  GLOG_LIBRARY)
