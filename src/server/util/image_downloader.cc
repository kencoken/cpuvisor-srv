#import "image_downloader.h"

namespace cpuvisor {

  ImageDownloader::ImageDownloader(const std::string& download_base_dir,
                                   boost::shared_ptr<PostProcessor> post_processor)
    : download_base_dir_(download_base_dir)
    , launch_queue_(new ImfileQueue())
    , post_processor_(post_processor)
    , postprocess_queue_(new ImfileQueue()) {

    // launch bunch of threads for initiating downloads
    for (size_t i = 0; i < 30; ++i) {
      launch_threads_.add_thread(new boost::thread(&ImageDownloader::run_launch_, this));
    }

    // launch post-processing thread
    postprocess_thread_.reset(new boost::thread(&ImageDownloader::run_postprocess_, this));
  }

  ImageDownloader::~ImageDownloader() {
    // interrupt post-processing thread to ensure termination before auto-detaching
    postprocess_thread_->interrupt();
    // and those in thread group
    launch_threads_.interrupt_all();
  }

  void ImageDownloader::downloadUrls(const std::vector<std::string>& urls,
                                     const std::string& tag,
                                     void* extra_data,
                                     boost::shared_ptr<DownloadCompleteCallback> callback) {

    DLOG(INFO) << "In image donwloader...";
    const std::string callback_hash = callback->hash();
    image_count_[callback_hash] = 0;

    for (size_t i = 0; i < urls.size(); ++i) {

      ImfileIfo imfile_ifo = prepareForDownload_(urls[i], tag, extra_data, callback);

      image_count_[callback_hash] += 1;

      // issue request asynchronously
      // (to be handled by download_stream_handler callback)
      // by adding to launch_queue_
      launch_queue_->push(imfile_ifo);

    }
  }

  // -----------------------------------------------------------------------------

  ImfileIfo ImageDownloader::prepareForDownload_(const std::string& url,
                                                 const std::string& tag,
                                                 void* extra_data,
                                                 boost::shared_ptr<DownloadCompleteCallback> callback) {

    // extract file ext from url
    std::string file_ext = fs::extension(fs::path(url));
    if (file_ext.empty()) {
      file_ext = ".jpg";
    }

    // generate id for filename
    static boost::uuids::random_generator uuid_gen = boost::uuids::random_generator();
    std::string id = boost::lexical_cast<std::string>(uuid_gen());

    // compose output dir
    fs::path out_dir_fs;
    if (tag.empty()) {
      out_dir_fs = fs::path(download_base_dir_);
    } else {
      out_dir_fs = fs::path(download_base_dir_) / fs::path(tag);
    }

    // ensure output dir exists
    if (!fs::exists(out_dir_fs)) {
      fs::create_directories(out_dir_fs);
    }

    // compose full output fname
    std::string full_path = (out_dir_fs /
                             fs::path(id + file_ext)).string();

    // create object
    ImfileIfo imfile_ifo;
    imfile_ifo.url = url;
    imfile_ifo.fname = full_path;
    imfile_ifo.extra_data = extra_data;
    imfile_ifo.callback = callback;

    return imfile_ifo;

  }

  void ImageDownloader::run_launch_() {

    http::client::options options;
    options.follow_redirects(true);

    http::client client(options);

    while(true) {

      ImfileIfo imfile_ifo;
      launch_queue_->waitAndPop(imfile_ifo);

      http::client::request request(imfile_ifo.url);
      request << net::header("Connection", "close");
      http::client::response response;
      response = client.get(request, body_handler(postprocess_queue_,
                                                  imfile_ifo));
      DLOG(INFO) << "Issued GET request for URL: " << imfile_ifo.url;

    }

  }

  void ImageDownloader::run_postprocess_() {

    while (true) {

      ImfileIfo imfile_ifo;
      postprocess_queue_->waitAndPop(imfile_ifo);

      const std::string callback_hash = imfile_ifo.callback->hash();

      // process an image
      if (imfile_ifo.completed) {
        if (post_processor_) {
          post_processor_->process(imfile_ifo.fname,
                                   imfile_ifo.extra_data);
        }
        image_count_[callback_hash] -= 1;
      } else {
        image_count_[callback_hash] -= 1;
      }

      // check to see if associated callback (if any) should be called
      int32_t images_remaining = image_count_[callback_hash];
      CHECK_GE(images_remaining, 0);

      if (images_remaining == 0) {
        (*imfile_ifo.callback)();
      }

    }

  }
}
