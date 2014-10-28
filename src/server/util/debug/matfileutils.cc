#include "matfileutils.h"

// matio 1.5.1+ support
#define MEM_CONSERVE MAT_F_DONT_COPY_DATA
  
// specific public functions for writing scalars
int matfileutils::writeFloat(mat_t* out, const char* name, const float data) {
  matio_classes mclass = MAT_C_SINGLE;
  matio_types mtype = MAT_T_SINGLE;
  return writeMatrix_(out, name, &data, 1, 1, mclass, mtype);
}

int matfileutils::writeDouble(mat_t* out, const char* name, const double data) {
  matio_classes mclass = MAT_C_DOUBLE;
  matio_types mtype = MAT_T_DOUBLE;
  return writeMatrix_(out, name, &data, 1, 1, mclass, mtype);
}

int matfileutils::writeInt(mat_t* out, const char* name, const int data) {
  matio_classes mclass = MAT_C_INT32;
  matio_types mtype = MAT_T_INT32;
  return writeMatrix_(out, name, &data, 1, 1, mclass, mtype);
}

int matfileutils::writeString(mat_t* out, const char* name, const char* data) {
  matio_classes mclass = MAT_C_CHAR;
  matio_types mtype = MAT_T_UINT8;
  return writeMatrix_(out, name, data, 1, strlen(data), mclass, mtype);
}

// specific public functions for writing matrices

int matfileutils::writeFloatMat(mat_t* out, const char* name, const float* data,
				const size_t ndim, const size_t mdim) {
  matio_classes mclass = MAT_C_SINGLE;
  matio_types mtype = MAT_T_SINGLE;
  // flip dimensions of matrix
  // (to ensure compatibility between C row major vs MATLAB column major,
  // all matrices will be inversed)
  size_t ndim_r = mdim, mdim_r = ndim;
  return writeMatrix_(out, name, data, ndim_r, mdim_r, mclass, mtype);
}

int matfileutils::writeDoubleMat(mat_t* out, const char* name, const double* data,
				 const size_t ndim, const size_t mdim) {
  matio_classes mclass = MAT_C_DOUBLE;
  matio_types mtype = MAT_T_DOUBLE;
  // flip dimensions of matrix
  // (to ensure compatibility between C row major vs MATLAB column major,
  // all matrices will be inversed)
  size_t ndim_r = mdim, mdim_r = ndim;
  return writeMatrix_(out, name, data, ndim_r, mdim_r, mclass, mtype);
}

int matfileutils::writeIntMat(mat_t* out, const char* name, const int* data,
			      const size_t ndim, const size_t mdim) {
  matio_classes mclass = MAT_C_INT32;
  matio_types mtype = MAT_T_INT32;
  // flip dimensions of matrix
  // (to ensure compatibility between C row major vs MATLAB column major,
  // all matrices will be inversed)
  size_t ndim_r = mdim, mdim_r = ndim;
  return writeMatrix_(out, name, data, ndim_r, mdim_r, mclass, mtype);
}

int matfileutils::writeUInt8Mat(mat_t* out, const char* name, const uint8_t* data,
				const size_t ndim, const size_t mdim) {
  matio_classes mclass = MAT_C_UINT8;
  matio_types mtype = MAT_T_UINT8;
  size_t ndim_r = mdim, mdim_r = ndim;
  return writeMatrix_(out, name, data, ndim_r, mdim_r, mclass, mtype);
}

// generic private function for writing data
int matfileutils::writeMatrix_(mat_t* out, const char* name, const void* data,
			       const size_t ndim, const size_t mdim,
			       const matio_classes mclass, const matio_types mtype) {
  int retval;
  matvar_t* matvar;
  size_t dims[2] = {ndim,mdim};

  matvar = Mat_VarCreate(name, mclass, mtype, 2, dims, (void*)data, MEM_CONSERVE);
  retval = Mat_VarWrite(out, matvar, MAT_COMPRESSION_NONE);
  Mat_VarFree(matvar);

  return retval;
}

// write vector of strings
int matfileutils::writeVectOfStrs(mat_t* out, const char* name,
				  std::vector<std::string>& data) {
  matvar_t** matvar;
  matvar_t* cell_matvar;
  size_t dims[2] = {1,1};

  int data_sz = data.size(), data_sz_str;
  char** substr = new char*[data_sz];

  matvar = new matvar_t*[data_sz];

  for (int i = 0; i < data_sz; ++i) {
    data_sz_str = data[i].size() + 1;
    substr[i] = new char[data_sz_str];

    // copy std::string to a c string before writing
    std::copy(data[i].begin(), data[i].end(), substr[i]);
    substr[i][data[i].size()] = '\0';

    dims[1] = strlen(substr[i]);
    // create matvar to contain the substring
    matvar[i] = Mat_VarCreate("data", MAT_C_CHAR, MAT_T_UINT8, 2, dims,
			      substr[i], MEM_CONSERVE);
  }

  dims[0] = data_sz;
  dims[1] = 1;
  // finally, create the cell array
  cell_matvar = Mat_VarCreate(name, MAT_C_CELL, MAT_T_CELL, 2, dims, matvar,
			      MEM_CONSERVE);

  int retval = Mat_VarWrite(out, cell_matvar, MAT_COMPRESSION_NONE);

  // following line also frees vars in matvar array, even if using MEM_CONSERVE
  // (so there is no need to free separately - doing so causes a double memory
  // free error)
  Mat_VarFree(cell_matvar);
  // free allocated memory for strings
  for (int i = 1; i < data_sz; ++i) {
    delete[] substr[i];
  }
  delete[] substr;
  //delete[] matvar; // this causes a double free for some reason

  return retval;
}

// specific public functions for reading scalars
int matfileutils::readFloat(mat_t* in, const char* name, float* const data) {
  size_t ndim, mdim;
  float* tmpdata;
  int retval = createAndReadFloatMat(in, name, &tmpdata, &ndim, &mdim);
  if (!retval) {
    if ((ndim != 1) || (mdim != 1)) {
      retval = MF_TYPEERR;
    }
  }
  (*data) = (*tmpdata);
  delete[] tmpdata;

  return retval; 
}

int matfileutils::readDouble(mat_t* in, const char* name, double* const data) {
  size_t ndim, mdim;
  double* tmpdata;
  int retval = createAndReadDoubleMat(in, name, &tmpdata, &ndim, &mdim);
  if (!retval) {
    if ((ndim != 1) || (mdim != 1)) {
      retval = MF_TYPEERR;
    }
  }
  (*data) = (*tmpdata);
  delete[] tmpdata;

  return retval; 
}

int matfileutils::readInt(mat_t* in, const char* name, int* const data) {
  size_t ndim, mdim;
  int* tmpdata;
  int retval = createAndReadIntMat(in, name, &tmpdata, &ndim, &mdim);
  if (!retval) {
    if ((ndim != 1) || (mdim != 1)) {
      retval = MF_TYPEERR;
    }
  }
  (*data) = (*tmpdata);
  delete[] tmpdata;

  return retval; 
}

int matfileutils::createAndReadString(mat_t* in, const char* name, char** data) {    
  size_t ndim, mdim;
  matio_classes mclass;
  matio_types mtype;
  unsigned short int* data_short;
  int retval = createAndReadMatrix_<unsigned short int>(in, name, &data_short,
							&ndim, &mdim, &mclass, &mtype);

  size_t longdim, shortdim;
  if (ndim > mdim) {
    longdim = ndim;
    shortdim = mdim;
  } else {
    longdim = mdim;
    shortdim = ndim;
  }
  
  (*data) = new char[ndim*mdim];
  for (int i = 0; i < longdim; ++i) {
    (*data)[i] = static_cast<char>(data_short[i] & 0xFF);
  }
  delete[] data_short;

  if (!retval){
    if ((mclass != MAT_C_CHAR) ||// (mtype != MAT_T_UINT8) ||
	(shortdim != 1) || (longdim != strlen(*data))) {
      retval = MF_TYPEERR;
    }
  }

  return retval;
}

// specific public functions for reading matrices
// ALL output arrays must be preallocated before call
// can find dimensions using getMatDims function

int matfileutils::createAndReadFloatMat(mat_t* in, const char* name, float** data,
					size_t* ndim, size_t* mdim) {
  matio_classes mclass;
  matio_types mtype;
  int retval = createAndReadMatrix_<float>(in, name, data,
					   ndim, mdim, &mclass, &mtype);

  if (!retval){
    if (mclass != MAT_C_SINGLE) {// || (mtype != MAT_T_SINGLE)) {
      retval = MF_TYPEERR;
    }
  }

  return retval;
}

int matfileutils::createAndReadDoubleMat(mat_t* in, const char* name, double** data,
					 size_t* ndim, size_t* mdim) {
  matio_classes mclass;
  matio_types mtype;
  int retval = createAndReadMatrix_<double>(in, name, data,
					    ndim, mdim, &mclass, &mtype);

  if (!retval){
    if (mclass != MAT_C_DOUBLE) {// || (mtype != MAT_T_DOUBLE)) {
      retval = MF_TYPEERR;
    }
  }

  return retval;
}

int matfileutils::createAndReadIntMat(mat_t* in, const char* name, int** data,
				      size_t* ndim, size_t* mdim) {
  matio_classes mclass;
  matio_types mtype;
  int retval = createAndReadMatrix_<int>(in, name, data,
					 ndim, mdim, &mclass, &mtype);

  if (!retval){
    if (mclass != MAT_C_INT32) {//|| (mtype != MAT_T_INT32)) {
      retval = MF_TYPEERR;
    }
  }

  return retval;
}

int matfileutils::readMatDims(mat_t* in, const char* name, size_t* ndim, size_t* mdim) {
  matvar_t* matvar;

  matvar = Mat_VarReadInfo(in, (char*)name);

  if (matvar == 0) return MF_LOADERR;

  // reverse matrix dimensions to be consistent with all other
  // functions
  // (conversion from matlab column-major to c row-major)
  (*mdim) = matvar->dims[0];
  (*ndim) = matvar->dims[1];

  return MF_NOERR;
}

// generic private function for reading data
// the data array must be deallocated by DELETE[] in the caller
template<typename T>
int matfileutils::createAndReadMatrix_(mat_t* in, const char* name, T** data,
				       size_t* ndim, size_t* mdim,
				       matio_classes* mclass, matio_types* mtype) {
  matvar_t* matvar;
  int start[2]={0,0}, stride[2]={1,1}, edge[2];

  matvar = Mat_VarReadInfo(in, (char*)name);

  if (matvar == 0) return MF_LOADERR;

  // calculate the size in bytes of the type contained in the matrix
  int mat_data_size = Mat_VarGetSize(matvar);
  int mat_type_sz = mat_data_size/(matvar->dims[0]*matvar->dims[1]);

  // check the size in bytes of the type is correct
  // (note: more thorough checks for type are done in the wrapper funcs)
  if (mat_type_sz != sizeof(T)) return MF_TYPEERR;

  (*ndim) = matvar->dims[0];
  (*mdim) = matvar->dims[1];
  (*mclass) = static_cast<matio_classes>(matvar->class_type);
  (*mtype) = static_cast<matio_types>(matvar->data_type);

  edge[0] = matvar->dims[0];
  edge[1] = matvar->dims[1];

  // allocate data
  (*data) = new T[(*ndim)*(*mdim)];

  // read in the data
  Mat_VarReadData(in, matvar, static_cast<void*>(*data), start, stride, edge);

  // flip the dimensions to make the matrix C-style row major
  int tmp = (*ndim);
  (*ndim) = (*mdim);
  (*mdim) = tmp;

  return MF_NOERR;
}

// read vector of strings from file
int matfileutils::readVectOfStrs(mat_t* in, const char* name,
				 std::vector<std::string>* data) {
  matvar_t** matvar;
  matvar_t* cell_matvar;

  cell_matvar = Mat_VarReadInfo(in, (char*)name);

  int retval = MF_NOERR;

  // check for errors
  if (cell_matvar == 0) return MF_LOADERR;
  //if ((cell_matvar->dims[0] == 1) && (cell_matvar->dims[1] == 1))
  //  retval = MF_DIMERR;
  //   if ((cell_matvar->class_type != MAT_C_CELL) ||
  //	(cell_matvar->data_type != MAT_T_CELL))
  if (cell_matvar->class_type != MAT_C_CELL)
    retval = MF_TYPEERR;

  if (!retval) { // if no errors
    // prepare variables for read
    int start[2]={0,0}, stride[2]={1,1}, edge[2];
    /* set maxdim to maximum dimension */
    int maxdim =
      (cell_matvar->dims[0] < cell_matvar->dims[1]) ?
      cell_matvar->dims[1] : cell_matvar->dims[0];
    /* preallocate vector output */
    data->resize(maxdim);
    /* load in cells */
    matvar = Mat_VarGetCellsLinear(cell_matvar, 0, 1, maxdim);
    if (matvar == 0) {
      retval = MF_LOADERR;
    }
    if (!retval) {
      /* loop over cells */
      for (int i = 0; i < maxdim; ++i) {
	// check for errors again (this time cell)
	
	//if ((matvar[i]->class_type != MAT_C_CHAR) ||
	//    (matvar[i]->data_type != MAT_T_UINT8))
	if (matvar[i]->class_type != MAT_C_CHAR)
	  retval = MF_TYPEERR;
	if (matvar[i]->dims[0] != 1)
	  retval = MF_TYPEERR;

	if (!retval) { // if no errors
	  // read in each string
	  /* allocate string */
	  char* substr = new char[matvar[i]->dims[1]+1];
	  edge[0] = matvar[i]->dims[0];
	  edge[1] = matvar[i]->dims[1];
	  retval = Mat_VarReadData(in, matvar[i], static_cast<void*>(substr),
				   start, stride, edge);
	  substr[matvar[i]->dims[1]] = '\0';

	  // convert c string back to std::string
	  (*data)[i] = std::string(substr);

	  delete[] substr;
	}
      
	if (retval) break; // on error break early
      }
    }
  }

  if (cell_matvar) Mat_VarFree(cell_matvar);

  //printf("Clearing matvar array\n");
  //free(matvar); // use free to clear matvar as it was declared in matio (which uses malloc)
  // FOR SOME REASON THE ABOVE CAUSES A SEGV

  return retval;

}

