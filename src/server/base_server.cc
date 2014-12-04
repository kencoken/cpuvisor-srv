#include "base_server.h"

#include <fstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <boost/thread.hpp>

#include "server/util/io.h"
#include "server/util/feat_util.h"

#ifdef MATEXP_DEBUG
  #include "server/util/debug/matfileutils_cpp.h"
#endif

namespace cpuvisor {

  void BaseServerPostProcessor::process(const std::string imfile,
                                        boost::shared_ptr<ExtraDataWrapper> extra_data) {

    // get feats matrix to extend from extra_data
    BaseServerExtraData* extra_data_s =
      dynamic_cast<BaseServerExtraData*>(extra_data.get());
    if (!extra_data_s) return; // return if query_ifo cannot be retrieved
    boost::shared_ptr<QueryIfo>& query_ifo = extra_data_s->query_ifo;
    boost::shared_ptr<StatusNotifier>& notifier = extra_data_s->notifier;
    if (query_ifo->state != QS_DATACOLL) {
      LOG(INFO) << "Skipping computing feature(s) for query " << query_ifo->id << " as it has advanced past data collection stage";
      return;
    }

    cv::Mat& feats = query_ifo->data.pos_feats;
    std::vector<std::string>& feat_paths = query_ifo->data.pos_paths;
    boost::mutex& feat_mutex = query_ifo->data.pos_mutex;

    cv::Mat feat;

    try {
      // callback function which computes a caffe encoding for a
      // downloaded image
      cv::Mat im = cv::imread(imfile, CV_LOAD_IMAGE_COLOR);
      im.convertTo(im, CV_32FC3);

      std::vector<cv::Mat> ims;
      ims.push_back(im);
      feat = encoder_.compute(ims);

    } catch (featpipe::InvalidImageError& e) {
      // delete image here
      DLOG(INFO) << "Removing invalid image: " << imfile;
      fs::remove(imfile);
      return;
    }

    if (!feats.empty()) {
      CHECK_EQ(feats.cols, feat.cols);
    }

    if (query_ifo->state != QS_DATACOLL) {
      LOG(INFO) << "Skipping adding feature(s) for query " << query_ifo->id << " as it has advanced past data collection stage";
      return;
    }

    DLOG(INFO) << "Pushing with sizes: " << feats.rows << "x" << feats.cols
               << " and " << feat.rows << "x" << feat.cols;
    {
      boost::mutex::scoped_lock lock(feat_mutex);
      feats.push_back(feat); // probably not thread safe, so locking here
      feat_paths.push_back(imfile);
    }

    DLOG(INFO) << "Feats size is now: " << feats.rows << "x" << feats.cols;

    // notify!
    notifier->post_image_processed_(query_ifo->id, imfile);
  }

  void BaseServerCallback::operator()() {
    LOG(INFO) << "Setting status of query to QS_DATACOLL_COMPLETE";
    if (query_ifo_->state == QS_DATACOLL) {
      query_ifo_->state = QS_DATACOLL_COMPLETE;
    }

    notifier_->post_all_images_processed_(query_ifo_->id);
    notifier_->post_state_change_(query_ifo_->id, query_ifo_->state);
  }

  // -----------------------------------------------------------------------------

  BaseServer::BaseServer(const cpuvisor::Config& config) {
    LOG(INFO) << "Initialize encoder...";
    const cpuvisor::CaffeConfig& caffe_config = config.caffe_config();
    encoder_.reset(new featpipe::CaffeEncoder(caffe_config));

    LOG(INFO) << "Load in features...";

    const cpuvisor::PreprocConfig preproc_config = config.preproc_config();

    CHECK(cpuvisor::readFeatsFromProto(preproc_config.dataset_feats_file(),
                                       &dset_feats_, &dset_paths_));
    dset_base_path_ = preproc_config.dataset_im_base_path();

    CHECK(cpuvisor::readFeatsFromProto(preproc_config.neg_feats_file(),
                                       &neg_feats_, &neg_paths_));
    neg_base_path_ = preproc_config.neg_im_base_path();

    post_processor_ =
      boost::shared_ptr<BaseServerPostProcessor>(new BaseServerPostProcessor(*encoder_));

    const cpuvisor::ServerConfig server_config = config.server_config();

    image_cache_path_ = server_config.image_cache_path();
    image_downloader_ =
      boost::shared_ptr<ImageDownloader>(new ImageDownloader(image_cache_path_,
                                                             post_processor_));

    notifier_ = boost::shared_ptr<StatusNotifier>(new StatusNotifier());

  }

  std::string BaseServer::startQuery(const std::string& tag) {
    static boost::uuids::random_generator uuid_gen = boost::uuids::random_generator();

    std::string id;
    do {
      LOG(INFO) << "Geneating UUID for Query ID";
      id = boost::lexical_cast<std::string>(uuid_gen());
    } while (queries_.find(id) != queries_.end());

    if (!tag.empty()) {
      DLOG(INFO) << "Starting query with tag: " << tag << " (" << id << ")";
    } else {
      DLOG(INFO) << "Starting query (" << id << ")";
    }
    boost::shared_ptr<QueryIfo> query_ifo(new QueryIfo(id, tag));
    queries_[id] = query_ifo;

    notifier_->post_state_change_(id, query_ifo->state);

    return id;
  }

  void BaseServer::setTag(const std::string& id, const std::string& tag) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    query_ifo->tag = tag;
  }

  void BaseServer::addTrs(const std::string& id, const std::vector<std::string>& urls) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (urls.size() == 0) throw InvalidRequestError("No urls specified in urls array");
    if (query_ifo->tag.empty()) throw InvalidRequestError("Calling addTrs before setting tag");

    if (query_ifo->state != QS_DATACOLL) {
      LOG(INFO) << "Skipping adding url(s) for query " << id << " as it has advanced past data collection stage";
      return;
    }

    boost::shared_ptr<BaseServerCallback>
      callback_obj(new BaseServerCallback(query_ifo, notifier_));

    boost::shared_ptr<BaseServerExtraData> extra_data(new BaseServerExtraData());
    extra_data->query_ifo = query_ifo;
    extra_data->notifier = notifier_; // to allow for notifications
                                      // from postproc callback

    image_downloader_->downloadUrls(urls, query_ifo->tag, extra_data,
                                    callback_obj);

  }

  void BaseServer::trainAndRank(const std::string& id, const bool block,
                                Ranking* ranking) {
    if ((!block) && (ranking)) {
      throw CannotReturnRankingError("Cannot retrieve ranked list directly unless block = true");
    }

    train(id, block);
    rank(id, block);

    if (ranking) {
      CHECK_EQ(block, true);

      (*ranking) = getRanking(id);
    }
  }

  void BaseServer::train(const std::string& id, const bool block) {
    if (block) {
      train_(id);
    } else {
      boost::thread proc_thread(&BaseServer::train_, this, id);
    }
  }

  void BaseServer::rank(const std::string& id, const bool block) {
    if (block) {
      rank_(id);
    } else {
      boost::thread proc_thread(&BaseServer::rank_, this, id);
    }
  }

  Ranking BaseServer::getRanking(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (query_ifo->state != QS_RANKED) {
      throw WrongQueryStatusError("Cannot test unles state = QS_TRAINED");
    }

    CHECK(!query_ifo->data.ranking.scores.empty());

    return query_ifo->data.ranking;
  }

  void BaseServer::freeQuery(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    size_t num_erased = queries_.erase(id);

    if (num_erased == 0) throw InvalidRequestError("Tried to free query which does not exist");
  }

  // Legacy methods --------------------------------------------------------------

  void BaseServer::addTrsFromFile(const std::string& id,
                                  const std::vector<std::string>& paths,
                                  const bool block) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (paths.size() == 0) throw InvalidRequestError("No paths specified in paths array");

    if (query_ifo->state != QS_DATACOLL) {
      LOG(INFO) << "Skipping adding paths(s) for query " << id << " as it has advanced past data collection stage";
      return;
    }

    if (block) {
      addTrsFromFile_(id, paths);
    } else {
      boost::thread proc_thread(&BaseServer::addTrsFromFile_, this, id, paths);
    }
  }

  void BaseServer::saveAnnotations(const std::string& id, const std::string& filename) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    LOG(INFO) << "Saving annotation file to: " << filename << "...";

    std::vector<std::string>& pos_paths = query_ifo->data.pos_paths;

    std::ofstream annofile(filename.c_str());

    size_t base_path_len = image_cache_path_.length();
    if (image_cache_path_[base_path_len-1] != fs::path("/").make_preferred().string()[0]) {
      base_path_len += 1;
    }

    for (size_t fi = 0; fi < pos_paths.size(); ++fi) {

      // get relative path from full path (a bit of a hack below!)
      std::string rel_path = pos_paths[fi];

      std::string train_img_searchstr = "postrainimgs"; // MAGIC CONSTANT
      size_t train_img_searchstr_idx = pos_paths[fi].find(train_img_searchstr);
      if (train_img_searchstr_idx != std::string::npos) {
        // relative path after searchstr if found
        size_t start_idx = train_img_searchstr_idx;// + train_img_searchstr.length() + 1;

        rel_path = pos_paths[fi].substr(start_idx, pos_paths[fi].length() - start_idx);
      } else {
        // else relative to cache path
        rel_path = pos_paths[fi].substr(base_path_len,
                                        pos_paths[fi].length() - base_path_len);
        CHECK_EQ(pos_paths[fi][base_path_len-1],
                 fs::path("/").make_preferred().string()[0]);
      }

      std::string anno = "+1";
      std::string from_dataset = "-1";
      std::string featfile = "";
      annofile << rel_path << " "
               << anno << " "
               << from_dataset << " "
               << featfile << "\n";

      DLOG(INFO) << "save_anno_file:" << fi << ": " << rel_path << " " << anno;
      DLOG(INFO) << "               " << pos_paths[fi];
      DLOG(INFO) << "               " << base_path_len << ":" << image_cache_path_;
    }

    annofile.flush();
  }

  void BaseServer::loadAnnotations(const std::string& filename,
                                   std::vector<std::string>* paths,
                                   std::vector<int32_t>* annos) {
    LOG(INFO) << "Loading annotation file from: " << filename << "...";

    std::ifstream annofile(filename.c_str());
    std::string line;

    (*paths) = std::vector<std::string>();
    (*annos) = std::vector<int32_t>();

    while (std::getline(annofile, line)) {
      if (!line.empty()) {
        std::string path;
        int32_t anno;

        std::vector<std::string> elems;
        // split fields
        boost::split(elems, line, boost::is_any_of("\t "));
        // validate input line
        if (elems.size() < 2) {
          throw InvalidAnnoFileError("Could not parse required fields from line of annotation file: " + line);
        }
        // first element is string ID
        path = elems[0];
        // second element is annotation generally +1/-1/0
        if ((elems[1] == "1") || (elems[1] == "+1")) {
          anno = 1;
        } else if (elems[1] == "0") {
          anno = 0;
        } else if (elems[1] == "-1") {
          anno = -1;
        } else {
          throw InvalidAnnoFileError("Unrecognised annotation");
        }

        DLOG(INFO) << "load_anno_file: " << path << " " << anno;
        paths->push_back(path);
        annos->push_back(anno);
      }
    }
  }

  // Protected methods -----------------------------------------------------------

  boost::shared_ptr<QueryIfo> BaseServer::getQueryIfo_(const std::string& id) {
    if (id.empty()) throw InvalidRequestError("No query id specified");

    std::map<std::string, boost::shared_ptr<QueryIfo> >::iterator query_iter =
      queries_.find(id);

    if (query_iter == queries_.end()) throw InvalidRequestError("Could not find query id");

    return query_iter->second;
  }

  void BaseServer::train_(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if ((query_ifo->state != QS_DATACOLL) && (query_ifo->state != QS_DATACOLL_COMPLETE)) {
      throw WrongQueryStatusError("Cannot train unles state = QS_DATACOLL or QS_DATACOLL_COMPLETE");
    }

    LOG(INFO) << "Entered train in correct state";
    query_ifo->state = QS_TRAINING;
    notifier_->post_state_change_(id, query_ifo->state);

    #ifndef NDEBUG // DEBUG
    DLOG(INFO) << "Will train with features computed from the following positive paths:";
    for (size_t i = 0; i < query_ifo->data.pos_paths.size(); ++i) {
      DLOG(INFO) << i+1 << ": " << query_ifo->data.pos_paths[i];
    }
    #endif

    query_ifo->data.model =
      cpuvisor::trainLinearSvm(query_ifo->data.pos_feats, neg_feats_);

    #ifdef MATEXP_DEBUG // DEBUG
    MatFile mat_file("prebasetrain.mat", true);
    mat_file.writeFloatMat("w_vect", (float*)query_ifo->data.model.data,
      query_ifo->data.model.rows,
      query_ifo->data.model.cols);
    #endif

    query_ifo->state = QS_TRAINED;
    notifier_->post_state_change_(id, query_ifo->state);
  }

  void BaseServer::rank_(const std::string& id) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    if (query_ifo->state != QS_TRAINED) {
      throw WrongQueryStatusError("Cannot rank unles state = QS_TRAINED");
    }

    LOG(INFO) << "Entered rank in correct state";
    query_ifo->state = QS_RANKING;
    notifier_->post_state_change_(id, query_ifo->state);

    cpuvisor::rankUsingModel(query_ifo->data.model,
                             dset_feats_,
                             &query_ifo->data.ranking.scores,
                             &query_ifo->data.ranking.sort_idxs);

    query_ifo->state = QS_RANKED;
    notifier_->post_state_change_(id, query_ifo->state);
  }

  void BaseServer::addTrsFromFile_(const std::string& id,
                                   const std::vector<std::string>& paths) {
    boost::shared_ptr<QueryIfo> query_ifo = getQueryIfo_(id);

    boost::shared_ptr<BaseServerExtraData> extra_data(new BaseServerExtraData());
    extra_data->query_ifo = query_ifo;
    extra_data->notifier = notifier_; // to allow for notifications
                                      // from postproc callback

    for (size_t i = 0; i < paths.size(); ++i) {
      post_processor_->process(paths[i], extra_data);
    }
  }

}
