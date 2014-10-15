#include "caffe_encoder.h"

#include <glog/logging.h>

//#define DEBUG_CAFFE_CHECKSUM

#ifdef DEBUG_CAFFE_CHECKSUM
#include <boost/crc.hpp>
#endif

cv::Mat featpipe::CaffeEncoder::compute(const std::vector<cv::Mat>& images) {

  cv::Mat features(images.size(), get_code_size(), CV_32FC1);

  for (size_t im_idx = 0; im_idx < images.size(); ++im_idx) {
    LOG(INFO) << "Prepare test images" << std::endl;
    std::vector<cv::Mat> caffe_images =
      augmentation_helper_.prepareImages(images[im_idx]);

    cv::Mat scores;

    {
      boost::lock_guard<boost::mutex> compute_lock(compute_mutex_);

      std::cout << "Copy test images to network" << std::endl;
      setNetTestImages(caffe_images, (*net_));

      std::cout << "Forwarding test images through network" << std::endl;
      const:: std::vector<caffe::Blob<float>*>& last_blobs = net_->ForwardPrefilled();

      caffe::Blob<float>* output_blob = 0;
      if (config_.output_blob_name == LAST_BLOB_STR) {
        output_blob = last_blobs[0];
      } else {
        LOG(INFO) << "Blob to be retrieved: '" << config_.output_blob_name << "'";
        const boost::shared_ptr<caffe::Blob<float> > blob =
          net_->blob_by_name(config_.output_blob_name);
        output_blob = const_cast<caffe::Blob<float>*>(blob.get());
      }

      std::cout << "Copy to output vector" << std::endl;
      scores = cv::Mat(output_blob->num(),
                       output_blob->count()/output_blob->num(),
                       CV_32FC1);
      switch (caffe::Caffe::mode()) {
      case caffe::Caffe::CPU: {
        DLOG(INFO) << "Copying from CPU";
        CHECK_EQ(output_blob->count(), scores.rows*scores.cols);

        caffe::caffe_copy(output_blob->count(), output_blob->cpu_data(),
                          (float*)scores.data);
        #ifdef DEBUG_CAFFE_CHECKSUM
        boost::crc_32_type result;
        result.process_bytes((float*)scores.data,
                             sizeof(float)*output_blob->count());
        DLOG(INFO) << "Copy from backend checksum for image: " << result.checksum();
        #endif
        } break;
      case caffe::Caffe::GPU:
        DLOG(INFO) << "Copying from GPU";
        caffe::caffe_copy(output_blob->count(), output_blob->gpu_data(),
                          (float*)scores.data);
        break;
      }

      double max_val, min_val;
      cv::minMaxLoc(scores, &min_val, &max_val);
      DLOG(INFO) << "scores size: " << scores.rows << " x " << scores.cols;
      DLOG(INFO) << "scores max: " << max_val;
      DLOG(INFO) << "scores min: " << min_val;
      DLOG(INFO) << "scores mean: " << cv::mean(scores);

    }

    cv::reduce(scores, features.row(im_idx), 0, CV_REDUCE_AVG);
  }

  std::cout << "Normalize features" << std::endl;
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
  // TODO - set net output # images

  augmentation_helper_ = AugmentationHelper(config_.mean_image_file);
  augmentation_helper_.aug_type = config_.data_aug_type;
}
