#include "matfileutils_cpp.h"

void MatFile::writeFloat(const std::string name, const float data) {
  handleError_(matfileutils::writeFloat(matfileh_, name.c_str(), data));
}
void MatFile::writeDouble(const std::string name, const double data) {
  handleError_(matfileutils::writeDouble(matfileh_, name.c_str(), data));
}
void MatFile::writeInt(const std::string name, const int data) {
  handleError_(matfileutils::writeInt(matfileh_, name.c_str(), data));
}
void MatFile::writeString(const std::string name, const char* data) {
  handleError_(matfileutils::writeString(matfileh_, name.c_str(), data));
}
void MatFile::writeFloatMat(const std::string name, const float* data,
			    const size_t ndim, const size_t mdim) {
  handleError_(matfileutils::writeFloatMat(matfileh_, name.c_str(), data,
					   ndim, mdim));
}
void MatFile::writeDoubleMat(const std::string name, const double* data,
			     const size_t ndim, const size_t mdim) {
  handleError_(matfileutils::writeDoubleMat(matfileh_, name.c_str(), data,
					    ndim, mdim));
}
void MatFile::writeIntMat(const std::string name, const int* data,
			  const size_t ndim, const size_t mdim) {
  handleError_(matfileutils::writeIntMat(matfileh_, name.c_str(), data,
					 ndim, mdim));
}
void MatFile::writeUInt8Mat(const std::string name, const uint8_t* data,
			    const size_t ndim, const size_t mdim) {
  handleError_(matfileutils::writeUInt8Mat(matfileh_, name.c_str(), data,
					   ndim, mdim));
}
void MatFile::writeVectOfStrs(const std::string name,
			      std::vector<std::string>& data) {
  handleError_(matfileutils::writeVectOfStrs(matfileh_, name.c_str(),
					     data));
}
float MatFile::readFloat(const std::string name) {
  float data;
  handleError_(matfileutils::readFloat(matfileh_, name.c_str(), &data));
  return data;
}
double MatFile::readDouble(const std::string name) {
  double data;
  handleError_(matfileutils::readDouble(matfileh_, name.c_str(), &data));
  return data;
}
int MatFile::readInt(const std::string name) {
  int data;
  handleError_(matfileutils::readInt(matfileh_, name.c_str(), &data));
  return data;
}
void MatFile::readString(const std::string name, std::string* data) {
  char* data_c;
  int errcode = matfileutils::createAndReadString(matfileh_,
						  name.c_str(),
						  &data_c);
  (*data) = std::string(data_c);
  delete[] data_c;
  handleError_(errcode);
}
void MatFile::readFloatMat(const std::string name, std::vector<float>* data,
			   size_t* ndim, size_t* mdim) {
  float* data_c;
  int errcode = matfileutils::createAndReadFloatMat(matfileh_,
						    name.c_str(),
						    &data_c,
						    ndim, mdim);
  (*data) = std::vector<float>(data_c,(data_c + ((*ndim)*(*mdim))));
  delete[] data_c;
  handleError_(errcode);
}
void MatFile::readDoubleMat(const std::string name, std::vector<double>* data,
			    size_t* ndim, size_t* mdim) {
  double* data_c;
  int errcode = matfileutils::createAndReadDoubleMat(matfileh_,
						     name.c_str(),
						     &data_c,
						     ndim, mdim);
  (*data) = std::vector<double>(data_c,(data_c + ((*ndim)*(*mdim))));
  delete[] data_c;
  handleError_(errcode);
}
void MatFile::readIntMat(const std::string name, std::vector<int>* data,
			 size_t* ndim, size_t* mdim) {
  int* data_c;
  int errcode = matfileutils::createAndReadIntMat(matfileh_,
						  name.c_str(),
						  &data_c,
						  ndim, mdim);
  (*data) = std::vector<int>(data_c,(data_c + ((*ndim)*(*mdim))));
  delete[] data_c;
  handleError_(errcode);
}

void MatFile::readVectOfStrs(const std::string name,
			     std::vector<std::string>* data) {
  handleError_(matfileutils::readVectOfStrs(matfileh_, name.c_str(), data));
}

void MatFile::readMatDims(const std::string name,
			  size_t* ndim, size_t* mdim) {
  handleError_(matfileutils::readMatDims(matfileh_, name.c_str(), ndim, mdim));
}

