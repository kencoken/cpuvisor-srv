#include "augmentation_helper.h"
#include <glog/logging.h>

//#define DEBUG_CAFFE_IMS

#ifdef DEBUG_CAFFE_IMS
#include <fstream>
#endif

using namespace featpipe;

std::vector<cv::Mat>
AugmentationHelper::prepareImages(const cv::Mat& image) {
  //const size_t IMAGE_DIM = image_dim;
  const size_t CROPPED_DIM = cropped_dim;
  std::vector<cv::Mat> output_ims;

  CHECK_EQ(image.channels(), 3);
  CHECK_EQ(image.depth(), CV_32F);

  DLOG(INFO) << "Preparing augmented images..." << std::endl;

  if (use_mean_image_) {
    CHECK_EQ(mean_image_.rows, CROPPED_DIM);
    CHECK_EQ(mean_image_.cols, CROPPED_DIM);
  }

  switch (aug_type) {
  case DAT_NONE:
    DLOG(INFO) << "Augmenting using whole image..." << std::endl;
    output_ims = augmentWholeImage(image);
    break;
  case DAT_ASPECT_CORNERS:
    DLOG(INFO) << "Augmenting using aspect corners..." << std::endl;
    output_ims = augmentAspectCorners(image);
    break;
  default:
    LOG(FATAL) << "Unsupported aug_type!";
  }

  DLOG(INFO) << "Done with augmentation" << std::endl;

  return output_ims;

}

std::vector<cv::Mat>
AugmentationHelper::augmentWholeImage(const cv::Mat& image) const {

  const size_t CROPPED_DIM = cropped_dim;
  std::vector<cv::Mat> output_ims;

  // extract centre crop
  DLOG(INFO) << "Get whole cropped image";
  cv::Mat cropped_im = getWholeCropCaffeImage(image, CROPPED_DIM);
  output_ims.push_back(cropped_im);

  #ifdef DEBUG_CAFFE_IMS
  // cv::imshow("Original Image", image / 255.0);
  // LOG(INFO) << "image: " << image.rows << "x" << image.cols;
  // cv::waitKey(0);
  // cv::destroyWindow("Original Image");
  // cv::imshow("Centre Crop", cropped_im / 255.0);
  // LOG(INFO) << "cropped_im: " << cropped_im.rows << "x" << cropped_im.cols;
  // cv::waitKey(0);
  // cv::destroyWindow("Centre Crop");
  #endif

  DLOG(INFO) << "Multiply base image";
  cropped_im *= image_mul;

  DLOG(INFO) << "image_mul: " << image_mul;

  if (use_mean_image_) {
    DLOG(INFO) << "Subtract mean from base image";
    cropped_im -= mean_image_;
  }

  return output_ims;

}

std::vector<cv::Mat>
AugmentationHelper::augmentAspectCorners(const cv::Mat& image) const {

  const size_t IMAGE_DIM = image_dim;
  const size_t CROPPED_DIM = cropped_dim;
  std::vector<cv::Mat> output_ims;

  // resize to IMAGE_DIM x N where IMAGE_DIM is the smaller dimension
  DLOG(INFO) << "Getting base Caffe image..." << std::endl;
  cv::Mat base_im = getBaseCaffeImage(image, IMAGE_DIM);

  for (size_t flip_idx = 0; flip_idx < 2; ++flip_idx) {
    if (flip_idx == 1) {
      // flip input image second time around (if applicable)
      cv::flip(base_im, base_im, 1);
    }

    #ifdef DEBUG_CAFFE_IMS
    // cv::imshow("Original Image", image / 255.0);
    // LOG(INFO) << "image: " << image.rows << "x" << image.cols;
    // cv::waitKey(0);
    // cv::destroyWindow("Original Image");
    // cv::imshow("Base Image", base_im / 255.0);
    // LOG(INFO) << "base_im: " << base_im.rows << "x" << base_im.cols;
    // cv::waitKey(0);
    // cv::destroyWindow("Base Image");
    #endif

    base_im *= image_mul;

    DLOG(INFO) << "Adding centre image... (" << flip_idx << ")" << std::endl;
    // add centre image
    {
      size_t start_idx_i = (base_im.rows - CROPPED_DIM)/2;
      size_t start_idx_j = (base_im.cols - CROPPED_DIM)/2;
      output_ims.push_back(cv::Mat::zeros(CROPPED_DIM, CROPPED_DIM, CV_32FC3));

      DLOG(INFO) << "IMAGE_DIM: " << IMAGE_DIM << std::endl;
      DLOG(INFO) << "CROPPED_DIM: " << CROPPED_DIM << std::endl;
      DLOG(INFO) << "base_im sz: " << base_im.rows << " x " << base_im.cols << std::endl;
      DLOG(INFO) << "start_idx: " << start_idx_i << ", " << start_idx_j << std::endl;

      cv::Mat cropped_im = output_ims.back();

      DLOG(INFO) << "Extracting centre crop..." << std::endl;
      base_im(cv::Rect(start_idx_j, start_idx_i, CROPPED_DIM, CROPPED_DIM)).copyTo(cropped_im);

      #ifdef DEBUG_CAFFE_IMS
      // cv::imshow("Centre Image", cropped_im / (image_mul*255.0));
      // LOG(INFO) << "cropped_im: " << cropped_im.rows << "x" << cropped_im.cols;
      // cv::waitKey(0);
      // cv::destroyWindow("Centre Image");
      #endif

      if (use_mean_image_) {
        DLOG(INFO) << "Subtracing mean image..." << std::endl;
        cropped_im -= mean_image_;
      }
    }

    DLOG(INFO) << "Adding corner images... (" << flip_idx << ")" << std::endl;
    // add corner images
    for (size_t i = 0; i < 4; ++i) {
      output_ims.push_back(cv::Mat::zeros(CROPPED_DIM, CROPPED_DIM, CV_32FC3));
      cv::Mat cropped_im = output_ims.back();
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

      #ifdef DEBUG_CAFFE_IMS
      // cv::imshow("Corner Image", cropped_im / (image_mul*255.0));
      // LOG(INFO) << "cropped_im: " << cropped_im.rows << "x" << cropped_im.cols;
      // cv::waitKey(0);
      // cv::destroyWindow("Corner Image");
      #endif

      if (use_mean_image_) {
        cropped_im -= mean_image_;
      }
    }

  }

  return output_ims;
}
