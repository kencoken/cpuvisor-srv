#include "preproc.h"

namespace cpuvisor {

  void procTextFile(const std::string& text_path,
                    const std::string& proto_path,
                    featpipe::CaffeEncoder& encoder,
                    const std::string& base_path,
                    const size_t limit) {
    std::vector<std::string> paths;

    std::ifstream imfiles(text_path);

    // std::string base_path;
    // std::getline(imfiles, base_path);
    fs::path base_path_fs(base_path);
    if (!base_path.empty()) {
      CHECK(fs::exists(base_path_fs) && fs::is_directory(fs::canonical(base_path_fs)))
        << "First line of file should be path to root of dataset or blank";
    }

    std::string imfile;
    while (std::getline(imfiles, imfile)) {
      if ((limit > 0) && (paths.size() >= limit)) break;
      if (!imfile.empty()) {
        paths.push_back(imfile);
      }
    }

    CHECK_GT(paths.size(), 0) << "No paths could be read from file: " << text_path;

    cv::Mat feats(paths.size(), encoder.get_code_size(), CV_32F);

    for (size_t i = 0; i < paths.size(); ++i) {

      LOG(INFO) << "Computing feature for image: " << paths[i];

      std::string full_path = (base_path_fs / fs::path(paths[i])).string();

      cv::Mat feat = cpuvisor::computeFeat(full_path, encoder);
      feat.copyTo(feats.row(i));
      //feats.row(i) = cpuvisor::computeFeat(full_path, encoder); <- this doesn't work

    }

    LOG(INFO) << "Writing features to: " << proto_path;
    cpuvisor::writeFeatsToProto(feats, paths, proto_path);
  }

}
