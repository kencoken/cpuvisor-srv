////////////////////////////////////////////////////////////////////////////
//    File:        generic_direct_encoder.h
//    Author:      Ken Chatfield
//    Description: Generic interface to direct feature extractor
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_GENERIC_DIRECT_ENCODER_H_
#define FEATPIPE_GENERIC_DIRECT_ENCODER_H_

#include <vector>
#include "opencv2/opencv.hpp"

namespace featpipe {

  class GenericDirectEncoder {
  public:
    /* empty virtual destructor, to allow overriding by derived
       classes */
    virtual ~GenericDirectEncoder() { }
    virtual GenericDirectEncoder* clone() const = 0;
    virtual cv::Mat compute(const std::vector<cv::Mat>& images) = 0;
    virtual size_t get_code_size() const = 0;

    virtual cv::Mat compute(cv::Mat& image) {
      std::vector<cv::Mat> images;
      images.push_back(image);

      return compute(images);
    }
  };
}

#endif
