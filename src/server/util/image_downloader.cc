#import "image_downloader.h"

namespace cpuvisor {

  ImageDownloader::ImageDownloader(const std::string& download_base_dir,
                                   PostProcessor* post_processor)
    : post_processor_(post_processor)
    , download_base_dir_(download_base_dir) {

    // launch post-processing thread
    postprocess_thread_->reset(new boost::thread(&ImageDownloader::run_postprocess_, this));
  }

  ImageDownloader::~ImageDownloader() {
    // interrupt post-processing thread to ensure termination before auto-detaching
    postprocess_thread_->interrupt();
  }

  void ImageDownloader::downloadUrls(const std::vector<std::string>& urls) {

  }

  // -----------------------------------------------------------------------------

  void ImageDownloader::run_postprocess_() {
    std::string imfile;
    postprocess_queue_.waitAndPop(imfile);
    // process an image
    if (post_processor_) {
      post_processor_->process(imfile);
    }
  }
}
