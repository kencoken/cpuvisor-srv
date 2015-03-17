#include "caffe_netinst.h"

#include <glog/logging.h>

namespace featpipe {

  cv::Mat CaffeNetInst::compute(const std::vector<cv::Mat>& images,
                                std::vector<std::vector<cv::Mat> >* _debug_input_images) {

    ready_ = false;

    try {
      cv::Mat feats;

      compute_(images, &feats, _debug_input_images);

      ready_ = true;
      if (ready_cond_var_) {
        ready_cond_var_->notify_all();
      }

      return feats;
    } catch(...) {
      ready_ = true;
      throw;
    }
  }

  size_t CaffeNetInst::get_code_size() const {
    if (config_.output_blob_name == LAST_BLOB_STR) {
      const std::vector<caffe::Blob<float>* >& output_blobs = net_->output_blobs();
      return output_blobs[0]->count() / output_blobs[0]->num();
    } else {
      boost::shared_ptr<caffe::Blob<float> > blob = net_->blob_by_name(config_.output_blob_name);
      return blob->count() / blob->num();
    }
  }

  void CaffeNetInst::initNetFromConfig_() {

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

    DLOG(INFO) << "Initializing network with " << image_count << " images";
    net_.reset(new caffe::Net<float>(config_.param_file.c_str(), caffe::TEST, image_count));
    net_->CopyTrainedLayersFrom(config_.model_file.c_str());

    augmentation_helper_ = AugmentationHelper(config_.mean_image_file);
    augmentation_helper_.aug_type = config_.data_aug_type;
  }

  void CaffeNetInst::compute_(const std::vector<cv::Mat>& images,
                              cv::Mat* feats,
                              std::vector<std::vector<cv::Mat> >* _debug_input_images) {

    if (_debug_input_images) {
      (*_debug_input_images) = std::vector<std::vector<cv::Mat> >();
    }

    size_t subbatch_sz = 0;

    std::vector<cv::Mat> flat_images;

    for (size_t im_idx = 0; im_idx < images.size(); ++im_idx) {
      std::vector<cv::Mat> subbatch = prepareImage_(images[im_idx]);

      if (im_idx == 0) {
        subbatch_sz = subbatch.size();
      } else {
        CHECK_EQ(subbatch_sz, subbatch.size());
      }

      flat_images.insert(flat_images.end(), subbatch.begin(), subbatch.end());
      if (_debug_input_images) {
        _debug_input_images->push_back(flat_images);
      }

    }

    cv::Mat scores = forwardPropImages_(flat_images);

    // decompose scores to feats again

    cv::Mat feats_mat(images.size(), scores.cols, CV_32FC1);
    (*feats) = feats_mat;

    for (size_t im_idx = 0; im_idx < images.size(); ++im_idx) {
      cv::Mat subfeats = scores(cv::Rect(0, subbatch_sz*im_idx,
                                         scores.cols, subbatch_sz));

      cv::reduce(subfeats, feats->row(im_idx), 0, CV_REDUCE_AVG);

      VLOG(1) << "Normalizing feature...";
      cv::normalize(feats->row(im_idx), feats->row(im_idx));
    }


  }

  std::vector<cv::Mat> CaffeNetInst::prepareImage_(const cv::Mat image) {

    //CHECK(!image.empty());

    cv::Mat in_image;

    if (config_.use_rgb_images) {
      LOG(INFO) << "Converting BGR image to RGB";
      cv::cvtColor(image, in_image, CV_BGR2RGB);
    } else {
      in_image = image;
    }

    //CHECK(!in_image.empty());

    #ifdef DEBUG_CAFFE_IMS // DEBUG
    cv::imshow("Original Image", in_image/255);
    cv::waitKey();
    #endif

    LOG(INFO) << "Prepare test images" << std::endl;
    std::vector<cv::Mat> out_images =
      augmentation_helper_.prepareImages(in_image);

    #ifdef DEBUG_CAFFE_IMS // DEBUG
    cv::imshow("Image", out_images[0]/255);
    cv::waitKey();
    cv::destroyAllWindows();
    #endif

    return out_images;

  }

  cv::Mat CaffeNetInst::forwardPropImages_(std::vector<cv::Mat> images) {
    cv::Mat scores;

    boost::lock_guard<boost::mutex> compute_lock(compute_mutex_);

    VLOG(1) << "Copying images to network for feature computation...";
    caffeutils::setNetTestImages(images, (*net_));

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

    return scores;

  }

}
