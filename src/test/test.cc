#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

std::string getCleanTempDir() {
  boost::filesystem::path cwd_fs(boost::filesystem::initial_path());
  boost::filesystem::path tempdir_fs = cwd_fs / boost::filesystem::path("tempdir");
  std::string tempdir = tempdir_fs.string();

  // remove temp directory if it exists
  boost::filesystem::remove_all(tempdir_fs);
  // then recreate
  boost::filesystem::create_directories(tempdir_fs);

  return tempdir;
}

std::string getTempFile(const std::string& tempdir) {
  boost::filesystem::path tempdir_fs(tempdir);

  // generate simple 'random' filename
  std::string tempfn;
  for (size_t i = 0; i < 6; ++i) {
    tempfn += boost::lexical_cast<std::string>(std::rand() % 10);
  }
  tempfn += ".file";

  boost::filesystem::path tempfn_fs(tempfn);
  boost::filesystem::path tempfn_full_fs = tempdir_fs / tempfn_fs;
  std::string tempfn_full = tempfn_full_fs.string();

  return tempfn_full;
}

void removeTempDir(const std::string& tempdir) {
  // remove temp directory if it exists
  boost::filesystem::path tempdir_fs(tempdir);
  boost::filesystem::remove_all(tempdir_fs);
}


#include "test_sets/feats.inl"
