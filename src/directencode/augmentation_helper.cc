#include "augmentation_helper.h"
#include <glog/logging.h>

//#define DEBUG_CAFFE_IMS

#ifdef DEBUG_CAFFE_IMS
#include <fstream>
#endif

using namespace featpipe;

std::vector<cv::Mat>
AugmentationHelper::prepareImages(const cv::Mat& image) {
  //const int IMAGE_DIM = image_dim;
  const int CROPPED_DIM = cropped_dim;
  std::vector<cv::Mat> output_ims;

  if (image.channels() != 3) {
    throw InvalidImageError("Image did not pass validation check");
  }
  CHECK_EQ(image.depth(), CV_32F);

  VLOG(1) << "Preparing augmented images..." << std::endl;

  if (use_mean_image_) {
    CHECK_EQ(mean_image_.rows, CROPPED_DIM);
    CHECK_EQ(mean_image_.cols, CROPPED_DIM);
  }

  switch (aug_type) {
  case DAT_NONE:
    VLOG(1) << "Augmenting using whole image..." << std::endl;
    output_ims = augmentWholeImage(image);
    break;
  case DAT_ASPECT_CORNERS:
    VLOG(1) << "Augmenting using aspect corners..." << std::endl;
    output_ims = augmentAspectCorners(image);
    break;
  default:
    LOG(FATAL) << "Unsupported aug_type!";
  }

  VLOG(1) << "Done with augmentation" << std::endl;

  return output_ims;

}

std::vector<cv::Mat>
AugmentationHelper::augmentWholeImage(const cv::Mat& image) const {

  const int CROPPED_DIM = cropped_dim;
  std::vector<cv::Mat> output_ims;

  // extract centre crop
  VLOG(1) << "Get whole cropped image";
  cv::Mat cropped_im = getWholeCropCaffeImage(image, CROPPED_DIM);

  VLOG(1) << "Multiply base image";
  cropped_im *= image_mul;

  VLOG(1) << "image_mul: " << image_mul;

  #ifdef DEBUG_CAFFE_IMS // DEBUG
  cv::imshow("Base Image", cropped_im/255);
  cv::waitKey();
  #endif

  if (use_mean_image_) {
    VLOG(1) << "Subtract mean from base image";
    cropped_im -= mean_image_;
    //cv::Mat norm_cropped_im;
    //cv::subtract(cropped_im, mean_image_, norm_cropped_im);
    //norm_cropped_im.copyTo(cropped_im);

    #ifdef DEBUG_CAFFE_IMS // DEBUG
    cv::imshow("Mean Image", mean_image_/255);
    cv::waitKey();
    #endif
  }

  output_ims.push_back(cropped_im);

  return output_ims;

}

std::vector<cv::Mat>
AugmentationHelper::augmentAspectCorners(const cv::Mat& image) const {

  const int IMAGE_DIM = image_dim;
  const int CROPPED_DIM = cropped_dim;
  std::vector<cv::Mat> output_ims;

  // resize to IMAGE_DIM x N where IMAGE_DIM is the smaller dimension
  VLOG(1) << "Getting base Caffe image..." << std::endl;
  cv::Mat base_im = getBaseCaffeImage(image, IMAGE_DIM);

  for (size_t flip_idx = 0; flip_idx < 2; ++flip_idx) {
    if (flip_idx == 1) {
      // flip input image second time around (if applicable)
      cv::flip(base_im, base_im, 1);
    }

    base_im *= image_mul;

    VLOG(1) << "Adding centre image... (" << flip_idx << ")" << std::endl;
    // add centre image
    {
      int start_idx_i = (base_im.rows - CROPPED_DIM)/2;
      int start_idx_j = (base_im.cols - CROPPED_DIM)/2;
      CHECK_GE(start_idx_i, 0);
      CHECK_GE(start_idx_j, 0);

      cv::Mat cropped_im = cv::Mat::zeros(CROPPED_DIM, CROPPED_DIM, CV_32FC3);

      VLOG(1) << "IMAGE_DIM: " << IMAGE_DIM << std::endl;
      VLOG(1) << "CROPPED_DIM: " << CROPPED_DIM << std::endl;
      VLOG(1) << "base_im sz: " << base_im.rows << " x " << base_im.cols << std::endl;
      VLOG(1) << "start_idx: " << start_idx_i << ", " << start_idx_j << std::endl;

      VLOG(1) << "Extracting centre crop..." << std::endl;
      base_im(cv::Rect(start_idx_j, start_idx_i, CROPPED_DIM, CROPPED_DIM)).copyTo(cropped_im);

      if (use_mean_image_) {
        VLOG(1) << "Subtracing mean image..." << std::endl;
        cropped_im -= mean_image_;
        // cv::Mat norm_cropped_im;
        // cv::subtract(cropped_im, mean_image_, norm_cropped_im);
        // norm_cropped_im.copyTo(cropped_im);
      }

      output_ims.push_back(cropped_im);
    }

    VLOG(1) << "Adding corner images... (" << flip_idx << ")" << std::endl;
    // add corner images
    for (size_t i = 0; i < 4; ++i) {
      cv::Mat cropped_im = cv::Mat::zeros(CROPPED_DIM, CROPPED_DIM, CV_32FC3);
      switch (i) {
      case 0:
        base_im(cv::Rect(0, 0, CROPPED_DIM, CROPPED_DIM)).copyTo(cropped_im);
        break;
      case 1:
        base_im(cv::Rect(0, base_im.rows - CROPPED_DIM,
                         CROPPED_DIM, CROPPED_DIM)).copyTo(cropped_im);
        break;
      case 2:
        base_im(cv::Rect(base_im.cols - CROPPED_DIM, 0,
                         CROPPED_DIM, CROPPED_DIM)).copyTo(cropped_im);
        break;
      case 3:
        base_im(cv::Rect(base_im.cols - CROPPED_DIM, base_im.rows - CROPPED_DIM,
                         CROPPED_DIM, CROPPED_DIM)).copyTo(cropped_im);
        break;
      }

      if (use_mean_image_) {
        cropped_im -= mean_image_;
        // cv::Mat norm_cropped_im;
        // cv::subtract(cropped_im, mean_image_, norm_cropped_im);
        // norm_cropped_im.copyTo(cropped_im);
      }

      output_ims.push_back(cropped_im);
    }

  }

  return output_ims;
}
