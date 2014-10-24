////////////////////////////////////////////////////////////////////////////
//    File:        liblinear.h
//    Author:      Ken Chatfield
//    Description: Linear SVM using Liblinear library
//
//    This class is NOT thread-safe
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_LIBLINEAR_H_
#define FEATPIPE_LIBLINEAR_H_

#include <stdexcept>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdlib.h> /* for malloc/free */
#include "linear.h" /* liblinear library */

#include "generic_svm.h"

namespace featpipe {

  class Liblinear : public GenericSvm {
  public:
    Liblinear();
    virtual ~Liblinear();
    virtual void train(const float* input, const size_t feat_dim, const size_t n,
                       std::vector<std::vector<size_t> > labels);
    virtual void test(const float* input, const size_t n,
                      size_t** est_label, float** scoremat) const;
    // setter/getter funcs for SVM parameters
    int get_solver_type() const;
    double get_eps() const;
    double get_c() const;
    void set_solver_type(const int solver_type);
    void set_eps(const double eps);
    void set_c(const double c);
    // getter funcs for model output
    size_t get_feat_dim() const;
    size_t get_num_classes() const;
    float* get_w() const;
  protected:
    // SVM parameters
    int solver_type_;
    double eps_;
    double c_;
    const double BIAS_MUL_;
    // variables relating to currently trained model
    size_t feat_dim_; /* dimensionality of features */
    size_t num_classes_; /* number of classes */
    float* w_; /* [num_classes x (feat_dim+1)] matrix of w vectors */
  };

}

// Constructor/destructor ------
inline featpipe::Liblinear::Liblinear(): BIAS_MUL_(1.0) {
  solver_type_ = L2R_L1LOSS_SVC_DUAL;
  eps_ = 0.1;
  c_ = 10.0;
  w_ = 0;
}

inline featpipe::Liblinear::~Liblinear() {
  if (w_) {
    delete[] w_;
  }
}

// Inline functions ------
inline int featpipe::Liblinear::get_solver_type() const {
  return solver_type_;
}

inline double featpipe::Liblinear::get_eps() const {
  return eps_;
}

inline double featpipe::Liblinear::get_c() const {
  return c_;
}

inline void featpipe::Liblinear::set_solver_type(const int solver_type) {
  solver_type_ = solver_type;
}

inline void featpipe::Liblinear::set_eps(const double eps) {
  eps_ = eps;
}

inline void featpipe::Liblinear::set_c(const double c) {
  c_ = c;
}

inline size_t featpipe::Liblinear::get_feat_dim() const {
  return feat_dim_;
}

inline size_t featpipe::Liblinear::get_num_classes() const {
  return num_classes_;
}

inline float* featpipe::Liblinear::get_w() const {
  return w_;
}

#endif
