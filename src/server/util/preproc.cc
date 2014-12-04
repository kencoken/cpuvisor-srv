#include "preproc.h"

#ifdef MATEXP_DEBUG
  #include "server/util/debug/matfileutils_cpp.h"
#endif

namespace cpuvisor {

  void procTextFile(const std::string& text_path,
                    const std::string& proto_path,
                    featpipe::CaffeEncoder& encoder,
                    const std::string& base_path,
                    const int64_t limit,
                    const int64_t start_idx,
                    const int64_t end_idx) {
    std::vector<std::string> paths;

    std::ifstream imfiles(text_path.c_str());

    // std::string base_path;
    // std::getline(imfiles, base_path);
    fs::path base_path_fs(base_path);
    if (!base_path.empty()) {
      CHECK(fs::exists(base_path_fs) && fs::is_directory(fs::canonical(base_path_fs)))
        << "Base path should exist or be blank: " << base_path;
    }

    std::string imfile;
    int64_t iter_count = 0;
    while (std::getline(imfiles, imfile)) {
      if ((start_idx > -1) && (iter_count < start_idx)) {
        DLOG(INFO) << "Skipping index: " << iter_count << " (< " << start_idx << ")";
        if (!imfile.empty()) ++iter_count;
        continue;
      }

      if ((limit > 0) && (paths.size() >= static_cast<size_t>(limit))) {
        DLOG(INFO) << "Breaking, paths.size(): " << paths.size() << " > limit of: " << limit;
        break;
      }
      if ((end_idx > -1) && (iter_count == end_idx)) {
        DLOG(INFO) << "Breaking, iter_count: " << iter_count << " == " << end_idx;
        break;
      }

      if (!imfile.empty()) {
        DLOG(INFO) << "Pushing back: " << imfile;
        paths.push_back(imfile);
        ++iter_count;
      }
      CHECK_GE(iter_count, 0);
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

    #ifdef MATEXP_DEBUG // DEBUG
    MatFile mat_file(proto_path + ".mat", true);
    mat_file.writeFloatMat("feats", (float*)feats.data, feats.rows, feats.cols);
    mat_file.writeVectOfStrs("paths", paths);
    #endif

    // ensure output dir exists
    fs::path proto_dir_fs = fs::path(proto_path).parent_path();
    if (!fs::exists(proto_dir_fs)) {
      fs::create_directories(proto_dir_fs);
    }

    LOG(INFO) << "Writing features to: " << proto_path;
    cpuvisor::writeFeatsToProto(feats, paths, proto_path);
  }

}
