////////////////////////////////////////////////////////////////////////////
//    File:        augmentation_helper.h
//    Author:      Ken Chatfield
//    Description: Augmentation Helper
////////////////////////////////////////////////////////////////////////////

#ifndef FEATPIPE_AUGMENTATION_HELPER_H_
#define FEATPIPE_AUGMENTATION_HELPER_H_

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

#include "directencode/caffe_encoder_utils.h"

namespace featpipe {

  enum DataAugType {DAT_NONE = 0, DAT_ASPECT_CORNERS = 1};

  class AugmentationHelper {
  public:
    DataAugType aug_type;
    size_t image_dim;
    size_t cropped_dim;
    float image_mul;

    AugmentationHelper(const std::string& mean_image_path = "")
      : aug_type(DAT_ASPECT_CORNERS)
      , image_dim(256)
      , cropped_dim(224)
      , image_mul(1.0F) {//image_mul(255.0F) {

      use_mean_image_ = (!mean_image_path.empty());
      if (use_mean_image_) {
        DLOG(INFO) << "Loading mean image file from: " << mean_image_path << std::endl;
        mean_image_ = loadMeanImageFile(mean_image_path);

        // take centre crop from mean image
        const size_t IMAGE_DIM = image_dim;
        const size_t CROPPED_DIM = cropped_dim;
        CHECK_EQ(mean_image_.rows, CROPPED_DIM);
        CHECK_EQ(mean_image_.cols, CROPPED_DIM);
        CHECK_EQ(mean_image_.channels(), 3);
        CHECK_EQ(mean_image_.depth(), CV_32F);

        CHECK_GT(IMAGE_DIM, CROPPED_DIM);
      }
    }

    virtual std::vector<cv::Mat> prepareImages(const cv::Mat& image);

  protected:
    bool use_mean_image_;
    cv::Mat mean_image_;

    virtual std::vector<cv::Mat> augmentWholeImage(const cv::Mat& imobj) const;
    virtual std::vector<cv::Mat> augmentAspectCorners(const cv::Mat& imobj) const;

  };

}

#endif
