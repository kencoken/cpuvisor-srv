////////////////////////////////////////////////////////////////////////////
//    File:        preproc.h
//    Author:      Ken Chatfield
//    Description: Preprocessing functions
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_UTILS_PREPROC_H_
#define CPUVISOR_UTILS_PREPROC_H_

#include <vector>
#include <string>
#include <fstream>

#include <glog/logging.h>

#include <opencv2/opencv.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "directencode/caffe_encoder.h"
#include "server/util/feat_util.h"
#include "server/util/io.h"

namespace cpuvisor {
  void procTextFile(const std::string& text_path,
                    const std::string& proto_path,
                    featpipe::CaffeEncoder& encoder,
                    const std::string& base_path = std::string(),
                    const int64_t limit = -1,
                    const int64_t start_idx = -1,
                    const int64_t end_idx = -1);
  void procPathList(const std::vector<std::string>& paths,
                    const std::string& proto_path,
                    featpipe::CaffeEncoder& encoder,
                    const std::string& base_path = std::string());
  void procPathListAppend(const std::vector<std::string>& new_paths,
                          const std::string& proto_path,
                          featpipe::CaffeEncoder& encoder,
                          cv::Mat* existing_feats,
                          std::vector<std::string>* existing_paths,
                          const std::string& base_path = std::string());


  cv::Mat procPaths_(const std::vector<std::string>& paths,
                     featpipe::CaffeEncoder& encoder,
                     const std::string& base_path);
  void writeFeatsToProto_(const cv::Mat feats,
                          const std::vector<std::string>& paths,
                          const std::string& proto_path);

}

#endif
