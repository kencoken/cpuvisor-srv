#include <glog/logging.h>
#include <gflags/gflags.h>
#include <vector>
#include <sstream>

#include <cmath>
#include <cstdlib>
#include <sys/wait.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <boost/lexical_cast.hpp>

#include "server/util/io.h"

#include "cpuvisor_config.pb.h"

DEFINE_string(config_path, "../config.prototxt", "Server config file");
DEFINE_bool(dsetfeats, true, "Compute dataset features");
DEFINE_bool(negfeats, true, "Compute negative training image features");

DEFINE_int64(chunk_sz, 1000, "Size of computation chunks");
DEFINE_uint64(mem_req, 5, "Memory required in GB for each computation process");
DEFINE_uint64(combine_mem_req, 100, "Memory required in GB to combine chunks");
DEFINE_int64(job_limit, 80, "Limit in the number of jobs to run simultaneously");

DEFINE_bool(launch, true, "Launch the child workers (if false do nothing)");

int main(int argc, char* argv[]) {

  const fs::path launch_path_fs = fs::initial_path();
  const std::string launch_path = launch_path_fs.string();

  google::InstallFailureSignalHandler();
  google::SetUsageMessage("Preprocessing for CPU Visor server (using sge cluster)");
  google::ParseCommandLineFlags(&argc, &argv, true);

  // construct log directories
  const fs::path log_path_fs = launch_path_fs / fs::path("preproc_logs");
  if (!fs::exists(log_path_fs)) {
    fs::create_directories(log_path_fs);
  }

  cpuvisor::Config config;
  cpuvisor::readProtoFromTextFile(FLAGS_config_path, &config);

  const cpuvisor::PreprocConfig& preproc_config = config.preproc_config();

  if (FLAGS_dsetfeats) {
    LOG(INFO) << "Starting dataset feature computation...";
    int64_t im_count = cpuvisor::getTextFileLineCount(preproc_config.dataset_im_paths());
    int64_t chunk_count = static_cast<int64_t>(std::ceil(static_cast<double>(im_count) /
                                                         static_cast<double>(FLAGS_chunk_sz)));

    CHECK_GT(chunk_count, 0);

    int64_t job_limit = FLAGS_job_limit;
    if (job_limit > chunk_count) job_limit = chunk_count;

    std::vector<pid_t> pids(chunk_count);
    int64_t active_workers = 0;
    for (size_t i = 0; i < static_cast<size_t>(chunk_count); ++i) {

      if ((job_limit > 0) && (active_workers >= job_limit)) {
        // wait for a job to complete before proceeding
        int child_exit_status;

        LOG(INFO) << "Waiting for worker to complete, as reached maximum of " << job_limit << " workers...";
        pid_t completed_pid = wait(&child_exit_status);
        if (!WIFEXITED(child_exit_status) || WEXITSTATUS(child_exit_status) != 0) {
          LOG(FATAL) << "Process "
                     << " (pid " << completed_pid << ") failed";
        }
        for (size_t scan_i = 0; scan_i < i; ++scan_i) {
          if (pids[scan_i] == completed_pid) {
            // mark completed pid as done, so we don't wait again later
            pids[scan_i] = 0;
            break;
          }
        }

        --active_workers;
      }

      LOG(INFO) << "Forking worker process " << i+1 << " of " << chunk_count;
      pids[i] = fork();
      ++active_workers;

      int64_t start_idx = i*FLAGS_chunk_sz;
      int64_t end_idx = (i+1)*FLAGS_chunk_sz;
      if (end_idx > im_count) {
        end_idx = im_count;
        CHECK_EQ(i+1, chunk_count);
      }

      std::string job_name = "preproc_dsetchunk_"
        + boost::lexical_cast<std::string>(start_idx) + "-"
        + boost::lexical_cast<std::string>(end_idx);
      fs::path stdout_path_fs = log_path_fs /
        fs::path(job_name + ".o$JOB_ID");
      fs::path stderr_path_fs = log_path_fs /
        fs::path(job_name + ".e$JOB_ID");

      if (pids[i] == 0) {
        // child process
        std::ostringstream command;
        command << "qsub";
        command << " -sync y";
        command << " -V";
        command << " -wd \"" << launch_path << "\"";
        command << " -N " << job_name;
        command << " -o \":" << stdout_path_fs.string() << "\"";
        command << " -e \":" << stderr_path_fs.string() << "\"";
        command << " -l h_vmem=" << FLAGS_mem_req << "G";
        command << " ./cpuvisor_preproc_sge_launcher.sh";
        command << " --config_path \"" << FLAGS_config_path << "\"";
        command << " --dsetfeats";
        command << " --nonegfeats";
        command << " --startidx " << start_idx;
        command << " --endidx " << end_idx;

        std::string command_str = command.str();
        DLOG(INFO) << "Launching qsub as: <" << command_str << ">";
        if (FLAGS_launch) {
          int retval = std::system(command_str.c_str());
          if (retval != 0) {
            LOG(ERROR) << "Worker ended with non-zero return code: " << retval;
          }
        }

        return 0;
      } else if (pids[i] < 0) {
        LOG(FATAL) << "Could not fork process!";
      }
    }

    // wait for all processes to complete
    for (size_t i = 0; i < pids.size(); ++i) {
      if (pids[i] == 0) continue;

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

      std::string job_name = "preproc_merge";
      fs::path stdout_path_fs = log_path_fs /
        fs::path(job_name + ".o$JOB_ID");
      fs::path stderr_path_fs = log_path_fs /
        fs::path(job_name + ".e$JOB_ID");

      if (proc_pid == 0) {
        // child process
        std::ostringstream command;
        command << "qsub";
        command << " -sync y";
        command << " -V";
        command << " -wd \"" << launch_path << "\"";
        command << " -N " << job_name;
        command << " -o \":" << stdout_path_fs.string() << "\"";
        command << " -e \":" << stderr_path_fs.string() << "\"";
        command << " -l h_vmem=" << FLAGS_combine_mem_req << "G";
        command << " ./cpuvisor_preproc_sge_launcher.sh";
        command << " --config_path \"" << FLAGS_config_path << "\"";

        std::string command_str = command.str();
        DLOG(INFO) << "Launching qsub as: <" << command_str << ">";
        if (FLAGS_launch) {
          int retval = std::system(command_str.c_str());
          if (retval != 0) {
            LOG(ERROR) << "Worker ended with non-zero return code: " << retval;
          }
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

    std::string job_name = "preproc_negims";
    fs::path stdout_path_fs = log_path_fs /
      fs::path(job_name + ".o$JOB_ID");
    fs::path stderr_path_fs = log_path_fs /
      fs::path(job_name + ".e$JOB_ID");

    if (proc_pid == 0) {
      // child process
      std::ostringstream command;
      command << "qsub";
      command << " -sync y";
      command << " -V";
      command << " -wd \"" << launch_path << "\"";
      command << " -N " << job_name;
      command << " -o \":" << stdout_path_fs.string() << "\"";
      command << " -e \":" << stderr_path_fs.string() << "\"";
      command << " -l h_vmem=" << FLAGS_mem_req << "G";
      command << " ./cpuvisor_preproc_sge_launcher.sh";
      command << " --config_path \"" << FLAGS_config_path << "\"";
      command << " --nodsetfeats";
      command << " --negfeats";

      std::string command_str = command.str();
      DLOG(INFO) << "Launching qsub as: <" << command_str << ">";
      if (FLAGS_launch) {
        int retval = std::system(command_str.c_str());
        if (retval != 0) {
          LOG(ERROR) << "Worker ended with non-zero return code: " << retval;
        }
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
