////////////////////////////////////////////////////////////////////////////
//    File:        feat_util.h
//    Author:      Ken Chatfield
//    Description: Helpers for working with Caffe features
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_UTILS_FEAT_UTIL_H_
#define CPUVISOR_UTILS_FEAT_UTIL_H_

#include <vector>
#include <string>

#include <glog/logging.h>

#include <opencv2/opencv.hpp>

#include "directencode/caffe_encoder.h"

namespace cpuvisor {

  cv::Mat computeFeat(const std::string& full_path,
                      featpipe::CaffeEncoder& encoder);

  cv::Mat trainLinearSvm(const cv::Mat pos_feats, const cv::Mat neg_feats,
                         const std::vector<std::string> _debug_pos_paths = std::vector<std::string>(),
                         const std::vector<std::string> _debug_neg_paths = std::vector<std::string>(),
                         const double svm_c = 1.0);
  cv::Mat trainLinearSvm(const cv::Mat pos_feats, const cv::Mat neg_feats,
                         const double svm_c = 1.0);

  void rankUsingModel(const cv::Mat model, const cv::Mat dset_feats,
                      cv::Mat* scores, cv::Mat* sortIdxs);

}

#endif
