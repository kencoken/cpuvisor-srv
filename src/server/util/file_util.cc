#include "file_util.h"

#include <glog/logging.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace cpuvisor {

  /**
   * Compute relative path
   *
   * This function computes the path of full_path relative to
   * base_path assuming that it is its child directory.
   *
   * \param full_path The original full path
   * \param base_path The directory to which the relative path should
   *                      be generated
   * \param[out] rel_path The relative path - simply set to full_path
   *                          if base_path is empty, or full_path is
   *                          not a child of base_path
   *
   * \return false if full_path was not a child of base_path, true otherwise.
   *
   */
  bool relativePath(const std::string& full_path, const std::string& base_path,
                    std::string* rel_path) {

    std::string& rel_path_ref = *rel_path;
    rel_path_ref = full_path;

    DLOG(INFO) << "base_path is: '" << base_path << "'";
    if (!base_path.empty()) {
      size_t base_path_idx = rel_path_ref.find(base_path);
      if (base_path_idx == std::string::npos) {
        return false;
      } else {
        size_t start_idx = base_path_idx + base_path.length();
        DLOG(INFO) << "Trimming rel_path using: " << base_path << " " << start_idx;
        rel_path_ref = rel_path_ref.substr(start_idx, rel_path_ref.length() - start_idx);
        if (rel_path_ref[0] == fs::path("/").make_preferred().string()[0]) {
          // trim leading dir separator
          rel_path_ref = rel_path_ref.substr(1, rel_path_ref.length() - 1);
        }
        DLOG(INFO) << "Trimmed rel_path is: " << rel_path_ref;
      }
    }

    return true;

  }

}
