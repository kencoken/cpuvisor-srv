#include <glog/logging.h>
#include <gflags/gflags.h>
#include <vector>
#include <sstream>

#include <cmath>
#include <cstdlib>
#include <sys/wait.h>

#include "server/util/io.h"

#include "cpuvisor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");
DEFINE_bool(dsetfeats, true, "Compute dataset features");
DEFINE_bool(negfeats, true, "Compute negative training image features");

DEFINE_int64(chunk_sz, 1000, "Size of computation chunks");
DEFINE_uint64(mem_req, 5, "Memory required in GB for each computation process");
DEFINE_uint64(combine_mem_req, 100, "Memory required in GB to combine chunks");

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();
  google::SetUsageMessage("Preprocessing for CPU Visor server (using sge cluster)");
  google::ParseCommandLineFlags(&argc, &argv, true);

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  const cpuvisor::PreprocConfig& preproc_config = config.preproc_config();

  if (FLAGS_dsetfeats) {
    LOG(INFO) << "Starting dataset feature computation...";
    int64_t im_count = cpuvisor::getTextFileLineCount(preproc_config.dataset_im_paths());
    int64_t chunk_count = static_cast<int64_t>(std::ceil(static_cast<double>(im_count) /
                                                         static_cast<double>(FLAGS_chunk_sz)));

    CHECK_GT(chunk_count, 0);

    std::vector<pid_t> pids(chunk_count);
    for (size_t i = 0; i < static_cast<size_t>(chunk_count); ++i) {
      LOG(INFO) << "Forking worker process " << i+1 << " of " << chunk_count;
      pids[i] = fork();

      int64_t start_idx = i*FLAGS_chunk_sz;
      int64_t end_idx = (i+1)*FLAGS_chunk_sz;
      if (end_idx > im_count) {
        end_idx = im_count;
        CHECK_EQ(i+1, chunk_count);
      }

      if (pids[i] == 0) {
        // child process
        std::ostringstream command;
        command << "qsub";
        command << " -l h_vmem=" << FLAGS_mem_req << "G";
        command << " ./cpuvisor_preproc";
        command << " --config_path \"" << FLAGS_config_path << "\"";
        command << " --dsetfeats";
        command << " --nonegfeats";
        command << " --startidx " << start_idx;
        command << " --endidx " << end_idx;

        std::string command_str = command.str();
        DLOG(INFO) << "Launching qsub as: <" << command_str << ">";
        int retval = std::system(command_str.c_str());
        if (retval != 0) {
          LOG(ERROR) << "Worker ended with non-zero return code: " << retval;
        }

        return 0;
      } else if (pids[i] < 0) {
        LOG(FATAL) << "Could not fork process!";
      }
    }

    // wait for all processes to complete
    for (size_t i = 0; i < pids.size(); ++i) {
      int child_exit_status;

      while (-1 == waitpid(pids[i], &child_exit_status, 0));
      if (!WIFEXITED(child_exit_status) || WEXITSTATUS(child_exit_status) != 0) {
        LOG(FATAL) << "Process " << i << " of " << pids.size()
                   << " (pid " << pids[i] << ") failed";
      }
    }

    // merge chunks into a single whole
    {
      LOG(INFO) << "Forking worker process for dataset feature chunk combination...";
      pid_t proc_pid = fork();

      if (proc_pid == 0) {
        // child process
        std::ostringstream command;
        command << "qsub";
        command << " -l h_vmem=" << FLAGS_combine_mem_req << "G";
        command << " ./cpuvisor_preproc";
        command << " --config_path \"" << FLAGS_config_path << "\"";

        std::string command_str = command.str();
        DLOG(INFO) << "Launching qsub as: <" << command_str << ">";
        int retval = std::system(command_str.c_str());
        if (retval != 0) {
          LOG(ERROR) << "Worker ended with non-zero return code: " << retval;
        }

        return 0;
      } else if (proc_pid < 0) {
        LOG(FATAL) << "Could not fork process!";
      }

      // wait for process to complete
      int child_exit_status;

      wait(&child_exit_status);
      if (!WIFEXITED(child_exit_status) || WEXITSTATUS(child_exit_status) != 0) {
        LOG(FATAL) << "Process "
                   << " (pid " << proc_pid << ") failed";
      }
    }
  }

  if (FLAGS_negfeats) {
    LOG(INFO) << "Forking worker process for negative image feature computation...";
    pid_t proc_pid = fork();

    if (proc_pid == 0) {
      // child process
      std::ostringstream command;
      command << "qsub";
      command << " -l h_vmem=" << FLAGS_mem_req << "G";
      command << " ./cpuvisor_preproc";
      command << " --config_path \"" << FLAGS_config_path << "\"";
      command << " --nodsetfeats";
      command << " --negfeats";

      std::string command_str = command.str();
      DLOG(INFO) << "Launching qsub as: <" << command_str << ">";
      int retval = std::system(command_str.c_str());
      if (retval != 0) {
        LOG(ERROR) << "Worker ended with non-zero return code: " << retval;
      }

      return 0;
    } else if (proc_pid < 0) {
      LOG(FATAL) << "Could not fork process!";
    }

    // wait for process to complete
    int child_exit_status;

    wait(&child_exit_status);
    if (!WIFEXITED(child_exit_status) || WEXITSTATUS(child_exit_status) != 0) {
      LOG(FATAL) << "Process "
                 << " (pid " << proc_pid << ") failed";
    }
  }

  return 0;
}
