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
#include <boost/filesystem.hpp>

#include "directencode/caffe_encoder.h"
#include "server/util/feat_util.h"
#include "server/util/io.h"

namespace fs = boost::filesystem;

namespace cpuvisor {
  void procTextFile(const std::string& text_path,
                    const std::string& proto_path,
                    featpipe::CaffeEncoder& encoder,
                    const std::string& base_path = std::string(),
                    const size_t limit = 0);
}

#endif
