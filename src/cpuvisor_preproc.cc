#include <glog/logging.h>
#include <gflags/gflags.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <boost/lexical_cast.hpp>

#include "directencode/caffe_encoder.h"
#include "server/util/preproc.h"
#include "server/util/io.h"

#include "visor_config.pb.h"
#include "cpuvisor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");
DEFINE_bool(dsetfeats, true, "Compute dataset features");
DEFINE_bool(negfeats, true, "Compute negative training image features");

DEFINE_int64(startidx, -1, "Starting index for dataset feature computation (inclusive index)");
DEFINE_int64(endidx, -1, "Ending index for dataset feature computation (exclusive index)");

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  gflags::SetUsageMessage("Preprocessing for CPU Visor server");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  visor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  const visor::PreprocConfig& preproc_config = config.preproc_config();
  const cpuvisor::CaffeConfig& caffe_config = config.GetExtension(cpuvisor::caffe_config);

  // update augmentation if preproc-specific aug type is defined
  cpuvisor::CaffeConfig caffe_config_upd = caffe_config;
  if (preproc_config.HasExtension(cpuvisor::data_aug_type_preproc) &&
      (caffe_config.data_aug_type() != preproc_config.GetExtension(cpuvisor::data_aug_type_preproc))) {
    caffe_config_upd.set_data_aug_type(preproc_config.GetExtension(cpuvisor::data_aug_type_preproc));
  }

  featpipe::CaffeEncoder encoder(caffe_config_upd);

  if (FLAGS_dsetfeats) {
    std::string feats_file = preproc_config.dataset_index_file();

    {
      fs::path feats_file_fs(feats_file);
      std::string feats_file_stem = (feats_file_fs.parent_path() / feats_file_fs.stem()).string();
      std::string feats_file_ext = feats_file_fs.extension().string();
      bool using_idxs = false;

      if (FLAGS_startidx > -1) {
        feats_file_stem = feats_file_stem + "_" +
          boost::lexical_cast<std::string>(FLAGS_startidx + 1);
        using_idxs = true;
      }
      if (FLAGS_endidx > -1) {
        feats_file_stem = feats_file_stem + "-" +
          boost::lexical_cast<std::string>(FLAGS_endidx);
        using_idxs = true;
      }
      if (using_idxs) {
        feats_file = feats_file_stem + feats_file_ext;
      }
    }
    DLOG(INFO) << "Feats file is: " << feats_file;

    if (fs::exists(feats_file)) {
      LOG(INFO) << "Skipping existing feature file!";
    } else {
      cpuvisor::procTextFile(preproc_config.dataset_im_paths(),
                             feats_file,
                             encoder,
                             preproc_config.dataset_im_base_path(),
                             -1, FLAGS_startidx, FLAGS_endidx);
    }
  }

  if (FLAGS_negfeats) {
    DLOG(INFO) << "Neg feats file is: " << preproc_config.GetExtension(cpuvisor::neg_index_file);

    if (fs::exists(preproc_config.GetExtension(cpuvisor::neg_index_file))) {
      LOG(INFO) << "Skipping existing feature file!";
    } else {
      cpuvisor::procTextFile(preproc_config.GetExtension(cpuvisor::neg_im_paths),
                             preproc_config.GetExtension(cpuvisor::neg_index_file),
                             encoder,
                             preproc_config.GetExtension(cpuvisor::neg_im_base_path));
    }
  }

  return 0;
}
