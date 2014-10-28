#include <glog/logging.h>
#include <gflags/gflags.h>

#include "directencode/caffe_encoder.h"
#include "server/util/preproc.h"
#include "server/util/io.h"

#include "cpuvisor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");
DEFINE_bool(dsetfeats, true, "Compute dataset features");
DEFINE_bool(negfeats, true, "Compute negative training image features");

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  google::SetUsageMessage("Preprocessing for CPU Visor server");
  google::ParseCommandLineFlags(&argc, &argv, true);

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
  featpipe::CaffeEncoder encoder(caffe_config);

  const cpuvisor::PreprocConfig& preproc_config = config.preproc_config();

  if (FLAGS_dsetfeats) {
    cpuvisor::procTextFile(preproc_config.dataset_im_paths(),
                           preproc_config.dataset_feats_file(),
                           encoder,
                           preproc_config.dataset_im_base_path());
  }

  if (FLAGS_negfeats) {
    cpuvisor::procTextFile(preproc_config.neg_im_paths(),
                           preproc_config.neg_feats_file(),
                           encoder,
                           preproc_config.neg_im_base_path());
  }

  return 0;
}
