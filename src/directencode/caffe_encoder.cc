#include "caffe_encoder.h"

#include <queue>
#include <glog/logging.h>

//#define DEBUG_CAFFE_CHECKSUM

#ifdef DEBUG_CAFFE_CHECKSUM
#include <boost/crc.hpp>
#endif

cv::Mat featpipe::CaffeEncoder::compute(const std::vector<cv::Mat>& images,
                                        std::vector<std::vector<cv::Mat> >* _debug_input_images) {

  cv::Mat feats;

  if (config_.batch_sz == 1) {

    compute_(images, &feats, _debug_input_images);

  } else {

    std::queue<cv::Mat> images_queue;
    for (size_t im_idx = 0; im_idx < images.size(); ++im_idx) {
      images_queue.push(images[im_idx]);
    }

    // add image to processing queue
    bool input_im_array_ready;
    size_t proc_idx;
    size_t proc_count;
    {
      boost::mutex::scoped_lock prelock(precomp_mutex_);

      CHECK_LT(input_im_array_.size(), config_.batch_sz);
      size_t slots_left = config_.batch_sz - input_im_array_.size();

      proc_idx = input_im_array_.size();
      proc_count = std::min(images_queue.size(), slots_left);
      for (size_t i = 0; i < proc_count; ++i) {
        input_im_array_.push_back(images_queue.front());
        images_queue.pop();
      }

      input_im_array_ready = (input_im_array_.size() == config_.batch_sz);
    }

    if (input_im_array_ready) {
      // notify if batch_sz images have been collected - start processing batch
      precomp_cond_var_.notify_one();
    }

    {
      boost::shared_lock<boost::shared_mutex> postlock(postcomp_mutex_);

      boost::mutex local_mutex;
      boost::mutex::scoped_lock locallock(local_mutex);
      // wait until batch has finished processing
      while (!batch_was_processed_) {
        postcomp_cond_var_.wait(locallock);
      }

      // extract relevant feature(s)
      feats = output_feat_array_(cv::Rect(0, proc_idx,
                                          output_feat_array_.cols,
                                          proc_count));
    }

  }

  return feats;

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

  // account for image batch size
  image_count *= config_.batch_sz;

  caffe::Caffe::set_mode(caffe::Caffe::CPU);
  caffe::Caffe::set_phase(caffe::Caffe::TEST);

  DLOG(INFO) << "Initializing network with " << image_count << " images";
  net_.reset(new caffe::Net<float>(config_.param_file.c_str(), image_count));
  net_->CopyTrainedLayersFrom(config_.model_file.c_str());

  augmentation_helper_ = AugmentationHelper(config_.mean_image_file);
  augmentation_helper_.aug_type = config_.data_aug_type;

  // start background processing service for batch operations if required
  if (compute_batch_thread_) {
    compute_batch_thread_->interrupt();
    compute_batch_thread_.reset();
  }

  if (config_.batch_sz > 1) {
    compute_batch_thread_ =
      boost::shared_ptr<boost::thread>(new boost::thread(&CaffeEncoder::computeProc_,
                                                         this));
  }
}

void featpipe::CaffeEncoder::computeProc_() {

  batch_was_processed_ = false;

  while (true) {

    boost::mutex::scoped_lock prelock(precomp_mutex_);

    // wait until batch_sz have been collected
    while (input_im_array_.size() < config_.batch_sz) {
      precomp_cond_var_.wait(prelock);
      boost::this_thread::interruption_point();
    }

    // encode current batch and flag results are ready
    compute_(input_im_array_, &output_feat_array_);
    boost::this_thread::interruption_point();

    batch_was_processed_ = true;

    // notify waiting compute calls batch has been processed
    postcomp_cond_var_.notify_all();

    {
      // wait for previous batch to finish processing before starting new batch
      boost::unique_lock<boost::shared_mutex> postlock(postcomp_mutex_);
    }

    boost::this_thread::interruption_point();

    // clear the input/output arrays
    input_im_array_.clear();
    output_feat_array_ = cv::Mat();
    batch_was_processed_ = false;

  }

}

void featpipe::CaffeEncoder::compute_(const std::vector<cv::Mat>& images,
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

std::vector<cv::Mat> featpipe::CaffeEncoder::prepareImage_(const cv::Mat image) {

  cv::Mat in_image;

  if (config_.use_rgb_images) {
    LOG(INFO) << "Converting BGR image to RGB";
    cv::cvtColor(image, in_image, CV_BGR2RGB);
  } else {
    in_image = image;
  }

  CHECK(!in_image.empty());

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

cv::Mat featpipe::CaffeEncoder::forwardPropImages_(std::vector<cv::Mat> images) {
  cv::Mat scores;

  boost::lock_guard<boost::mutex> compute_lock(compute_mutex_);

  VLOG(1) << "Copying images to network for feature computation...";
  setNetTestImages(images, (*net_));

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
