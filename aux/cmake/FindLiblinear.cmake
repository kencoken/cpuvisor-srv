#
# Try to find the Liblinear library and include path.
# Once done this will define
#
# LIBLINEAR_FOUND
# Liblinear_INCLUDE_DIRS
# Liblinear_LIBRARIES
#
# A custom location can be specified for the library using Liblinear_DIR
#

set (Liblinear_DIR "" CACHE STRING
  "Custom location of the root directory of a Liblinear installation")

find_path(Liblinear_INCLUDE_DIR
  NAMES linear.h
  PATHS ${Liblinear_DIR}
  DOC "The directory where linear.h resides"
  )#NO_DEFAULT_PATH
  #NO_CMAKE_ENVIRONMENT_PATH
  #NO_CMAKE_PATH
  #NO_SYSTEM_ENVIRONMENT_PATH
  #NO_CMAKE_SYSTEM_PATH)
#find_path(Liblinear_INCLUDE_DIR
#  NAMES linear.h
#  DOC "The directory where linear.h resides")

find_library(Liblinear_LIBRARY
  NAMES linear
  PATHS ${Liblinear_DIR}
  DOC "The Liblinear library"
  )#NO_DEFAULT_PATH
  #NO_CMAKE_ENVIRONMENT_PATH
  #NO_CMAKE_PATH
  #NO_SYSTEM_ENVIRONMENT_PATH
  #NO_CMAKE_SYSTEM_PATH)
#find_library(Liblinear_LIBRARY
#  NAMES linear
#  DOC "The Liblinear library")


# handle the QUIETLY and REQUIRED arguments and set LIBLINEAR_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Liblinear
  "Could NOT find Liblinear. Install to system include/lib directory, or declare Liblinear_DIR (either using -DLiblinear_DIR=<path> flag or via CMakeGUI/ccmake) to point to root directory of library."
  Liblinear_LIBRARY Liblinear_INCLUDE_DIR)

if (LIBLINEAR_FOUND)
  set(Liblinear_LIBRARIES ${Liblinear_LIBRARY})
  set(Liblinear_INCLUDE_DIRS ${Liblinear_INCLUDE_DIR})
endif(LIBLINEAR_FOUND)

mark_as_advanced(
  Liblinear_INCLUDE_DIR
  Liblinear_LIBRARY)
