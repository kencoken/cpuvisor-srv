////////////////////////////////////////////////////////////////////////////
//    File:        matfileutils.h
//    Author:      Ken Chatfield
//    Description: Utilities for reading/writing to Matlab MAT files using
//                 matio library
////////////////////////////////////////////////////////////////////////////

#ifndef MATFILEUTILS_H_
#define MATFILEUTILS_H_

#include <vector>
#include <string>
#include <string.h>
#include <matio.h>
#include <stdint.h>

namespace matfileutils {

  enum MATFILE_ERRCODES {MF_NOERR = 0, MF_TYPEERR = 74628, MF_DIMERR = 847362,
			 MF_LOADERR = 473927};

  int writeFloat(mat_t* out, const char* name, const float data);
  int writeDouble(mat_t* out, const char* name, const double data);
  int writeInt(mat_t* out, const char* name, const int data);
  int writeString(mat_t* out, const char* name, const char* data);
  int writeFloatMat(mat_t* out, const char* name, const float* data,
			   const size_t ndim, const size_t mdim);
  int writeDoubleMat(mat_t* out, const char* name, const double* data,
		     const size_t ndim, const size_t mdim);
  int writeIntMat(mat_t* out, const char* name, const int* data,
		  const size_t ndim, const size_t mdim);
  int writeUInt8Mat(mat_t* out, const char* name, const uint8_t* data,
		    const size_t ndim, const size_t mdim);
  int writeVectOfStrs(mat_t* out, const char* name,
			     std::vector<std::string>& data);

  int readFloat(mat_t* in, const char* name, float* const data);
  int readDouble(mat_t* in, const char* name, double* const data);
  int readInt(mat_t* in, const char* name, int* const data);
  int createAndReadString(mat_t* in, const char* name, char** data);
  int createAndReadFloatMat(mat_t* in, const char* name, float** data,
			    size_t* ndim, size_t* mdim);
  int createAndReadDoubleMat(mat_t* in, const char* name, double** data,
			     size_t* ndim, size_t* mdim);
  int createAndReadIntMat(mat_t* in, const char* name, int** data,
			    size_t* ndim, size_t* mdim);
  int readVectOfStrs(mat_t* in, const char* name,
		     std::vector<std::string>* data);
  int readMatDims(mat_t* in, const char* name, size_t* ndim, size_t* mdim);

  int writeMatrix_(mat_t* out, const char* name, const void* data,
		   const size_t ndim, const size_t mdim,
		   const matio_classes mclass, const matio_types mtype);
  template<typename T>
  int createAndReadMatrix_(mat_t* in, const char* name, T** data,
			   size_t* ndim, size_t* mdim,
			   matio_classes* mclass, matio_types* mtype);
}

#endif
