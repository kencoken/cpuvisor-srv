#
# Try to find the MATIO library and include path.
# Once done this will define
#
# MATIO_FOUND
# MATIO_INCLUDE_DIRS
# MATIO_LIBRARIES
# 

find_path(MATIO_INCLUDE_DIR
  NAMES matio.h
  DOC "The directory where matio.h resides")

find_library(MATIO_LIBRARY
  NAMES matio
  DOC "The MATIO library")

find_library(ZLIB_LIBRARY
  NAMES z
  DOC "The ZLib library")

find_library(HDF5_LIBRARY
  NAMES hdf5
  DOC "The HDF5 library")

# handle the QUIETLY and REQUIRED arguments and set MATIO_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MATIO DEFAULT_MSG MATIO_LIBRARY ZLIB_LIBRARY HDF5_LIBRARY MATIO_INCLUDE_DIR)

if (MATIO_FOUND)
  set(MATIO_LIBRARIES ${MATIO_LIBRARY} ${ZLIB_LIBRARY} ${HDF5_LIBRARY})
  set(MATIO_INCLUDE_DIRS ${MATIO_INCLUDE_DIR})
endif(MATIO_FOUND)

mark_as_advanced(
  MATIO_INCLUDE_DIR
  MATIO_LIBRARY
  ZLIB_LIBRARY
  HDF5_LIBRARY)
 
