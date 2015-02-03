#include "caffe_encoder.h"

#include <glog/logging.h>

//#define DEBUG_CAFFE_CHECKSUM

#ifdef DEBUG_CAFFE_CHECKSUM
#include <boost/crc.hpp>
#endif

cv::Mat featpipe::CaffeEncoder::compute(const std::vector<cv::Mat>& images,
                                        std::vector<cv::Mat>* _debug_input_images) {

  cv::Mat features(images.size(), get_code_size(), CV_32FC1);

  for (size_t im_idx = 0; im_idx < images.size(); ++im_idx) {

    const cv::Mat* im = 0;
    cv::Mat rgb_im;
    if (config_.use_rgb_images) {
      LOG(INFO) << "Converting BGR image to RGB";

      cv::cvtColor(images[im_idx], rgb_im, CV_BGR2RGB);

      im = &rgb_im;
    } else {
      im = &images[im_idx];
    }
    CHECK_NOTNULL(im);

    #ifdef DEBUG_CAFFE_IMS // DEBUG
    cv::imshow("Original Image", (*im)/255);
    cv::waitKey();
    #endif

    LOG(INFO) << "Prepare test images" << std::endl;
    std::vector<cv::Mat> caffe_images =
      augmentation_helper_.prepareImages(*im);

    #ifdef DEBUG_CAFFE_IMS // DEBUG
    cv::imshow("Image", caffe_images[0]/255);
    cv::waitKey();
    cv::destroyAllWindows();
    #endif

    if (_debug_input_images) {
      (*_debug_input_images) = caffe_images;
    }

    cv::Mat scores;

    {
      boost::lock_guard<boost::mutex> compute_lock(compute_mutex_);

      VLOG(1) << "Copying images to network for feature computation...";
      setNetTestImages(caffe_images, (*net_));

      VLOG(1) << "Forwarding test images through network...";
      const:: std::vector<caffe::Blob<float>*>& last_blobs = net_->ForwardPrefilled();
      VLOG(1) << "Done forwarding!";

      caffe::Blob<float>* output_blob = 0;
      if (config_.output_blob_name == LAST_BLOB_STR) {
        output_blob = last_blobs[0];
      } else {
        LOG(INFO) << "Blob to be retrieved: '" << config_.output_blob_name << "'";
        const boost::shared_ptr<caffe::Blob<float> > blob =
          net_->blob_by_name(config_.output_blob_name);
        output_blob = const_cast<caffe::Blob<float>*>(blob.get());
      }

      scores = cv::Mat(output_blob->num(),
                       output_blob->count()/output_blob->num(),
                       CV_32FC1);
      switch (caffe::Caffe::mode()) {
      case caffe::Caffe::CPU: {
        VLOG(1) << "Copying from CPU";
        CHECK_EQ(output_blob->count(), scores.rows*scores.cols);

        caffe::caffe_copy(output_blob->count(), output_blob->cpu_data(),
                          (float*)scores.data);

        #ifdef DEBUG_CAFFE_CHECKSUM // DEBUG
        boost::crc_32_type result;
        result.process_bytes((float*)scores.data,
                             sizeof(float)*output_blob->count());
        DLOG(INFO) << "Copy from backend checksum for image: " << result.checksum();
        #endif

        } break;
      case caffe::Caffe::GPU:
        VLOG(1) << "Copying from GPU";
        caffe::caffe_copy(output_blob->count(), output_blob->gpu_data(),
                          (float*)scores.data);
        break;
      }

      #ifndef NDEBUG // DEBUG
      double max_val, min_val;
      cv::minMaxLoc(scores, &min_val, &max_val);
      VLOG(1) << "scores size: " << scores.rows << " x " << scores.cols;
      VLOG(1) << "scores max: " << max_val;
      VLOG(1) << "scores min: " << min_val;
      VLOG(1) << "scores mean: " << cv::mean(scores);
      #endif

    }

    cv::reduce(scores, features.row(im_idx), 0, CV_REDUCE_AVG);
  }

  VLOG(1) << "Normalizing features...";
  for (size_t im_idx = 0; im_idx < images.size(); ++im_idx) {
    cv::normalize(features.row(im_idx), features.row(im_idx));
  }

  return features;
}




void featpipe::CaffeEncoder::initNetFromConfig_() {

  size_t image_count;

  switch (config_.data_aug_type) {
  case DAT_NONE:
    image_count = 1;
    break;
  case DAT_ASPECT_CORNERS:
    image_count = 10;
    break;
  default:
    LOG(FATAL) << "Unsupported aug_type!";
  }

  caffe::Caffe::set_mode(caffe::Caffe::CPU);
  caffe::Caffe::set_phase(caffe::Caffe::TEST);

  DLOG(INFO) << "Initializing network with " << image_count << " images";
  net_.reset(new caffe::Net<float>(config_.param_file.c_str(), image_count));
  net_->CopyTrainedLayersFrom(config_.model_file.c_str());

  augmentation_helper_ = AugmentationHelper(config_.mean_image_file);
  augmentation_helper_.aug_type = config_.data_aug_type;
}
