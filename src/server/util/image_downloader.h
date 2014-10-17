////////////////////////////////////////////////////////////////////////////
//    File:        image_downloader.h
//    Author:      Ken Chatfield
//    Description: Download image URLs using a threadpool
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_IMAGE_DOWNLOADER_H_
#define CPUVISOR_IMAGE_DOWNLOADER_H_

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include "server/util/concurrent_queue.h"

namespace cpuvisor {

  class PostProcessor {
  public:
    virtual void process(const std::string imfile) = 0;
  };

  class ImageDownloader : boost::noncopyable {
  public:
    ImageDownloader(const std::string& download_base_dir_,
                    PostProcessor* post_processor = 0);
    virtual ~ImageDownloader();

    virtual void downloadUrls(const std::vector<std::string>& urls);
  protected:
    std::string download_base_dir_;
    // post-processing related vars
    virtual void run_postprocess_();
    PostProcessor* post_processor_;
    featpipe::ConcurrentQueue<std::string> postprocess_queue_;
    boost::shared_ptr<boost::thread> postprocess_thread_;
  };
}

#endif
