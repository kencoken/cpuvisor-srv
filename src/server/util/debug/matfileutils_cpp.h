////////////////////////////////////////////////////////////////////////////
//    File:        matfileutils_cpp.h
//    Author:      Ken Chatfield
//    Description: Utilities for reading/writing to Matlab MAT files using
//                 matio library and RAII class
////////////////////////////////////////////////////////////////////////////

#ifndef MATFILEUTILS_CPP_H_
#define MATFILEUTILS_CPP_H_

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include "matfileutils.h"

class MatFile {
 public:
  MatFile(const std::string matfile, const bool write) {
    if (write) {
      matfileh_ = Mat_Create(matfile.c_str(), 0);
    } else {
      matfileh_ = Mat_Open(matfile.c_str(), MAT_ACC_RDONLY);
    }
    if (matfileh_ == 0) {
      std::ostringstream strstrm;
      strstrm << "Matfile: " << matfile << " could not be opened";
      throw std::runtime_error(strstrm.str());
    }
  }
  ~MatFile() { Mat_Close(matfileh_); }

  void writeFloat(const std::string name, const float data);
  void writeDouble(const std::string name, const double data);
  void writeInt(const std::string name, const int data);
  void writeString(const std::string name, const char* data);
  void writeFloatMat(const std::string name, const float* data,
		     const size_t ndim, const size_t mdim);
  void writeDoubleMat(const std::string name, const double* data,
		      const size_t ndim, const size_t mdim);
  void writeIntMat(const std::string name, const int* data,
		   const size_t ndim, const size_t mdim);
  void writeUInt8Mat(const std::string name, const uint8_t* data,
		     const size_t ndim, const size_t mdim);
  void writeVectOfStrs(const std::string name,
		       std::vector<std::string>& data);
  float readFloat(const std::string name);
  double readDouble(const std::string name);
  int readInt(const std::string name);
  void readString(const std::string name, std::string* data);
  void readFloatMat(const std::string name, std::vector<float>* data,
		    size_t* ndim, size_t* mdim);
  void readDoubleMat(const std::string name, std::vector<double>* data,
		     size_t* ndim, size_t* mdim);
  void readIntMat(const std::string name, std::vector<int>* data,
		  size_t* ndim, size_t* mdim);
  void readVectOfStrs(const std::string name,
		      std::vector<std::string>* data);
  void readMatDims(const std::string name, size_t* ndim, size_t* mdim);
 protected:
   mat_t* matfileh_;
   void handleError_(int errcode) {
     switch(errcode) {
     case matfileutils::MF_TYPEERR:
       throw std::runtime_error("MatFile: Type Error");
     case matfileutils::MF_DIMERR:
       throw std::runtime_error("MatFile: Dimension Error");
     case matfileutils::MF_LOADERR:
       throw std::runtime_error("MatFile: Load Error");
     }
     if (errcode) {
       throw std::runtime_error("MatFile: Unknown Error");
     }
   }
 private:
   /* prevent copying */
   MatFile(const MatFile& rhs) { }
   MatFile& operator=(const MatFile& rhs) {
     Mat_Close(rhs.matfileh_);
     return (*this);
   }
};

#endif

