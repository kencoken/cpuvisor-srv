////////////////////////////////////////////////////////////////////////////
//    File:        image_downloader.h
//    Author:      Ken Chatfield
//    Description: Download image URLs using a threadpool
////////////////////////////////////////////////////////////////////////////

#ifndef CPUVISOR_IMAGE_DOWNLOADER_H_
#define CPUVISOR_IMAGE_DOWNLOADER_H_

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/network/protocol/http/client.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/network/protocol/http/response.hpp>
#include <boost/network/protocol/http/client/connection/connection_delegate_factory.hpp>
#include <boost/network/protocol/http/traits/delegate_factory.hpp>
#include <boost/network/protocol/http/client/connection/async_normal.hpp>
namespace net = boost::network;
namespace http = boost::network::http;

#include <glog/logging.h>

#include "server/util/concurrent_queue.h"

namespace cpuvisor {

  // callback functors -------------------

  class ExtraDataWrapper {
  public:
    virtual ~ExtraDataWrapper() { } // make wrapper class virtual to
                                    // allow downcasts
  };

  class PostProcessor {
  public:
    virtual void process(const std::string imfile,
                         boost::shared_ptr<ExtraDataWrapper> extra_data = boost::shared_ptr<ExtraDataWrapper>()) = 0;
  };

  class DownloadCompleteCallback {
  public:
    inline DownloadCompleteCallback() {
      static boost::uuids::random_generator uuid_gen = boost::uuids::random_generator();
      hash_val_ = boost::lexical_cast<std::string>(uuid_gen());
    }
    inline std::string hash() { return hash_val_; }
    virtual void operator()() = 0;
  private:
    std::string hash_val_;
  };

  // typedefs for complete queue ---------

  class ImfileIfo {
  public:
    ImfileIfo(): completed(false) { }

    std::string url;
    std::string fname;
    boost::shared_ptr<ExtraDataWrapper> extra_data; // optional extra data
    boost::shared_ptr<DownloadCompleteCallback> callback; // optional associated completion callback

    bool completed;
    std::string err_msg;
  };

  typedef featpipe::ConcurrentQueue<ImfileIfo> ImfileQueue;

  // class definition --------------------

  class ImageDownloader : boost::noncopyable {
  public:
    ImageDownloader(const std::string& download_base_dir_,
                    boost::shared_ptr<PostProcessor> post_processor = boost::shared_ptr<PostProcessor>());
    virtual ~ImageDownloader();

    virtual void downloadUrls(const std::vector<std::string>& urls,
                              const std::string& tag,
                              boost::shared_ptr<ExtraDataWrapper> extra_data = boost::shared_ptr<ExtraDataWrapper>(),
                              boost::shared_ptr<DownloadCompleteCallback> callback = boost::shared_ptr<DownloadCompleteCallback>());
  protected:
    virtual ImfileIfo prepareForDownload_(const std::string& url,
                                          const std::string& tag,
                                          boost::shared_ptr<ExtraDataWrapper> extra_data,
                                          boost::shared_ptr<DownloadCompleteCallback> callback = boost::shared_ptr<DownloadCompleteCallback>());
    std::string download_base_dir_;
    // launching image download
    virtual void run_launch_();
    boost::shared_ptr<ImfileQueue> launch_queue_;
    boost::thread_group launch_threads_;
    // post-processing related vars
    virtual void run_postprocess_();
    boost::shared_ptr<PostProcessor> post_processor_;
    boost::shared_ptr<ImfileQueue> postprocess_queue_;
    boost::shared_ptr<boost::thread> postprocess_thread_;

    std::map<std::string, int32_t> image_count_;
  };

  // STREAM HANDLER FOR CPP-NETLIB LIBRARY --
  // ---------------------------------------------------------------
  // pushes downloaded files to postprocess_queue to be processed by
  // postprocess_thread in an ImageDownloader class instance (which in
  // turn will call the process function of an associated
  // PostProcessor class instance if attached)

  struct body_handler {
    explicit body_handler(boost::shared_ptr<ImfileQueue> postprocess_queue,
                          ImfileIfo imfile_ifo)
      : postprocess_queue_(postprocess_queue)
      , imfile_ifo_(imfile_ifo)
      , body("") {
      DLOG(INFO) << "Initializing stream handler...";
    }

    BOOST_NETWORK_HTTP_BODY_CALLBACK(operator(), range, error) {
      // in here, range is the Boost.Range iterator_range, and error is
      // the Boost.System error code.
      //DLOG(INFO) << "In stream handler callback";
      if (!error) {
        body.append(boost::begin(range), boost::end(range));
        //DLOG(INFO) << "Still downloading: " << imfile_ifo_.url;
        //DLOG(INFO) << "Extending stored body of size " << body.size() << " bytes to " << body.size();
      } else {
        if (error == boost::asio::error::eof) {
          // write image to file
          std::ofstream out_file;
          out_file.open(imfile_ifo_.fname.c_str(), std::ios::out | std::ios::binary);
          out_file << body;
          DLOG(INFO) << "Wrote image to file: " << imfile_ifo_.fname;

          // push filename to queue for post-processing
          imfile_ifo_.completed = true;
          postprocess_queue_->push(imfile_ifo_);
        } else {
          LOG(ERROR) << "An error occurred when downloading an image";
          // push filename to queue with error message
          imfile_ifo_.completed = false;
          std::ostringstream sstrm;
          sstrm << "Error code: " << error;
          imfile_ifo_.err_msg = sstrm.str();

          postprocess_queue_->push(imfile_ifo_);
        }
      }
    }
    boost::shared_ptr<ImfileQueue> postprocess_queue_;
    ImfileIfo imfile_ifo_;

    std::string body;
  };


}

#endif
