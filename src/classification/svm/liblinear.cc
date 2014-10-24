#include "liblinear.h"

void featpipe::Liblinear::train(const float* input, const size_t feat_dim,
                                const size_t n,
                                std::vector<std::vector<size_t> > labels) {
  size_t fi, ei, ci, li;

  // Save dimensionality of input
  feat_dim_ = feat_dim;
  num_classes_ = labels.size();

  problem svmp;

  svmp.l = n; // number of training features
  svmp.n = feat_dim+1; // dim of each feature (including bias)
  svmp.bias = BIAS_MUL_; // single bias term

  // Copy across feature vectors
  // -------
  /* each input feature has a separate feature node */
  //****************svmp.x = new feature_node*[n];
  svmp.x = (struct feature_node**)malloc(sizeof(struct feature_node*)*n);

  for (fi = 0; fi < n; ++fi) {
    /* count non-zero entries */
    size_t nnz = 0;
    for (ei = 0; ei < feat_dim; ++ei) {
      if (input[fi*feat_dim + ei] != 0.0) {
        ++nnz;
      }
    }
    /* copy non-zero entries across */
    //**************svmp.x[fi] = new feature_node[nnz+2];
    svmp.x[fi] = (struct feature_node*)malloc(sizeof(struct feature_node)*(nnz+2));
    size_t nzi = 0;
    for (ei = 0; ei < feat_dim; ++ei) {
      if (input[fi*feat_dim + ei] != 0.0) {
        svmp.x[fi][nzi].index = ei+1; // 1-indexed attributes
        svmp.x[fi][nzi].value = static_cast<double>(input[fi*feat_dim + ei]);
        ++nzi;
      }
    }
    /* add bias term */
    svmp.x[fi][nnz].index = feat_dim+1;
    svmp.x[fi][nnz].value = static_cast<double>(svmp.bias);
    /* add termination term */
    svmp.x[fi][nnz+1].index = -1;
  }

  // Prepare labels and model output array
  // -------
  //*********************svmp.y = new int[n];
  svmp.y = (double*)malloc(sizeof(double)*n);

  if (w_) {
    delete[] w_;
    w_ = 0;
  }
  w_ = new float[labels.size()*(feat_dim+1)];

  // Setup SVM parameters
  // -------
  parameter svm_params;

  svm_params.solver_type = solver_type_;
  svm_params.eps = eps_;
  svm_params.C = c_;
  svm_params.nr_weight = 0;
  //svm_params.nr_weight = 2;
  //svm_params.weight_label = new int[2];
  //svm_params.weight_label[0] = +1;
  //svm_params.weight_label[1] = -1;
  //svm_params.weight = new double[2];
  //svm_params.weight[0] = 1;
  //svm_params.weight[1] = 1;

  const char *error= check_parameter(&svmp, &svm_params);
  bool allOK = (error==NULL);

  // Train a single SVM for each class
  // -------
  if (allOK) {
    for (ci = 0; ci < labels.size(); ++ci) {
      /* initialize label for this class to -1 */
      for (fi = 0; fi < n; ++fi) {
        svmp.y[fi] = -1;
      }
      /* now loop through updating label for positive samples */
      for (li = 0; li < labels[ci].size(); ++li) {
        assert(labels[ci][li] >= 0);
        assert(labels[ci][li] < n);
        svmp.y[labels[ci][li]] = 1; // labels are 0-indexed
      }
      /* then train the SVM */
      model* svm = ::train(&svmp, &svm_params);

      /* determine whether the svm function sign should be reversed */
      int wmul = ((svmp.y[0] == -1) ? -1 : 1);

      /* store the SVM function */
      for (ei = 0; ei <= feat_dim; ++ei) { // index + bias term
        w_[ci*(feat_dim+1)+ei] =
          static_cast<float>(wmul*svm->w[ei]); // only one class
      }
      //w_[ci*(feat_dim+1)+feat_dim] = static_cast<float>(svm->bias);

      /* free the svm model */
      free_and_destroy_model(&svm);
    }
  }

  // Free memory
  // -------
  //destroy_param(&svm_params);

  //delete[] svmp.y;
  free(svmp.y);
  for (fi = 0; fi < n; ++fi) {
    //delete[] svmp.x[fi];
    free(svmp.x[fi]);
  }
  //delete[] svmp.x;
  free(svmp.x);

  if (!allOK) {
    std::ostringstream strstrm;
    strstrm << "Liblinear: check parameters error: " << error;
    throw std::runtime_error(strstrm.str());
  }
}

void featpipe::Liblinear::test(const float* input, const size_t n,
                               size_t** est_label, float** scoremat) const {
  size_t i, ci, fi, ei;

  // If there is no trained model, return with error
  // -------
  if (!w_) {
    throw std::runtime_error("no model trained");
  }
  // Otherwise continue...
  // -------
  /* initialize scoremat */
  for (i = 0; i < (num_classes_*n); ++i) {
    (*scoremat)[i] = 0.0;
  }
  /* test models for each class in turn
     returning a CxN matrix:
       w_       x input = scoremat
       [Cx(d+1)]  [dxN]   [CxN] */
  for (ci = 0; ci < num_classes_; ++ci) {
    for (fi = 0; fi < n; ++fi) {
      /* iterating over elements of output matrix */
      for (ei = 0; ei < (feat_dim_+1); ++ei) {
        /* doing matrix multiplication */
        if (ei < feat_dim_) {
          (*scoremat)[ci*n+fi] += w_[ci*(feat_dim_+1)+ei]*
            input[fi*feat_dim_+ei];
        } else {
          /* last row is bias term */
          (*scoremat)[ci*n+fi] += w_[ci*(feat_dim_+1)+ei];
        }
      }
    }
  }
  /* now calculate estimated label */
  for (fi = 0; fi < n; ++fi) {
    int maxidx = -1;
    float maxval = -10000;
    for (ci = 0; ci < num_classes_; ++ci) {
      if ((*scoremat)[ci*n+fi] > maxval) {
        maxval = (*scoremat)[ci*n+fi];
        maxidx = ci;
      }
    }
    (*est_label)[fi] = maxidx;
  }
}
