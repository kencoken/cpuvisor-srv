#ifndef FEATPIPE_CAFFE_ENCODER_UTILS_H_
#define FEATPIPE_CAFFE_ENCODER_UTILS_H_

#include "opencv2/opencv.hpp"

#include <vector>
#include <string>

#include "caffe/caffe.hpp"

namespace featpipe {
  cv::Mat loadMeanImageFile(const std::string& mean_image_file);
  cv::Mat downsizeToBound(const cv::Mat& src, const size_t min_size);
  cv::Mat getBaseCaffeImage(const cv::Mat& src, const size_t min_size);
  cv::Mat getWholeCropCaffeImage(const cv::Mat& src, const size_t crop_size);

  size_t checkImagesAgainstBlob(const std::vector<cv::Mat>& images,
                                const caffe::Blob<float>* blob);

  void setNetTestImages(const std::vector<cv::Mat>& images, caffe::Net<float>& net);
}

#endif
