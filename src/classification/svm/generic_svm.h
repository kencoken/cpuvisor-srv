////////////////////////////////////////////////////////////////////////////
//    File:        generic_svm.h
//    Author:      Ken Chatfield
//    Description: Generic interface to SVM
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_GENERIC_SVM_H_
#define FEATPIPE_GENERIC_SVM_H_

#include <vector>

namespace featpipe {

  class GenericSvm {
  public:
    virtual ~GenericSvm() { }
    // input - row-major array of training features
    // d - dimensionality of each feature
    // n - number of features
    // labels - vector (of length #classes) with the indices of the
    //          (0-indexed) features containing each class
    virtual void train(const float* input, const size_t d, const size_t n,
                       std::vector<std::vector<size_t> > labels) = 0;
    // input - row-major array of test features
    // n - number of features
    // est_label - OUTPUT - array of length n containing the
    //             predicted label for each class
    // scoremat - OUTPUT - matrix of size n x <#classes> containing
    //            the raw SVM classification score for each class
    virtual void test(const float* input, const size_t n,
                      size_t** est_label, float** scoremat) const = 0;
  };

}

#endif
