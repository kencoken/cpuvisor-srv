#
# Try to find the CUDAHeaders
# Once done this will define
#
# CUDAHeaders_FOUND
# CUDAHeaders_INCLUDE_DIRS
#
# Uses the CUDA_DIR environment variable
#

find_path(CUDAHeaders_INCLUDE_DIR
  NAMES cublas_v2.h
  PATH_SUFFIXES include
  PATHS $ENV{CUDA_DIR}
  DOC "The directory where cublas_v2.h resides")

# handle the QUIETLY and REQUIRED arguments and set CUDAHeaders_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CUDAHeaders
  "Could NOT find CUDAHeaders. Install to system include/lib directory, or ensure that the CUDA_DIR environment variable properly set."
  CUDAHeaders_INCLUDE_DIR)

if (CUDAHEADERS_FOUND)
  set(CUDAHeaders_INCLUDE_DIRS ${CUDAHeaders_INCLUDE_DIR})
endif(CUDAHEADERS_FOUND)

mark_as_advanced(
  CUDAHeaders_INCLUDE_DIR)
 
