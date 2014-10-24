#include "zmq_server.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <google/protobuf/text_format.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace cpuvisor {

  ZmqServer::ZmqServer(const cpuvisor::Config& config)
    : config_(config)
    , base_server_(new BaseServer(config))
    , monitor_add_trs_images_thread_(new boost::thread(&ZmqServer::monitor_add_trs_images_, this))
    , monitor_add_trs_complete_thread_(new boost::thread(&ZmqServer::monitor_add_trs_complete_, this)) {

  }

  ZmqServer::~ZmqServer() {
    // interrupt serve thread to ensure termination before auto-detaching
    if (serve_thread_) serve_thread_->interrupt();
    if (monitor_add_trs_images_thread_) monitor_add_trs_images_thread_->interrupt();
    if (monitor_add_trs_complete_thread_) monitor_add_trs_complete_thread_->interrupt();
  }

  void ZmqServer::serve(const bool blocking) {
    if (!serve_thread_) {
      serve_thread_.reset(new boost::thread(&ZmqServer::serve_, this));
    }
    if (blocking) {
      serve_thread_->join();
    }
  }

  // -----------------------------------------------------------------------------

  void ZmqServer::serve_() {
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    std::string server_endpoint = config_.server_config().server_endpoint();
    boost::replace_all(server_endpoint, "localhost", "*");
    socket.bind(server_endpoint.c_str());

    std::cout << "ZMQ server started..." << std::endl;

    while (true) {
      zmq::message_t request;

      //  Wait for next request from client
      socket.recv(&request);
      RPCReq rpc_req;
      rpc_req.ParseFromArray(request.data(), request.size());

      std::string rpc_req_str;
      google::protobuf::TextFormat::PrintToString(rpc_req, &rpc_req_str);
      std::cout << "Received Protobuf:\n" << "-----------------\n"
                << rpc_req_str << "-----------------\n" << std::endl;

      RPCRep rpc_rep = dispatch_(rpc_req);

      //  Send reply back to client
      std::string rpc_rep_serialized;
      rpc_rep.SerializeToString(&rpc_rep_serialized);

      //std::cout << "***********\n" << rpc_rep_serialized << std::endl << "***********\n" << std::endl;

      // for some reason, this results in corruption of binary data
      // zmq::message_t reply((void*)rpc_rep_serialized.c_str(),
      //                      rpc_rep_serialized.size(), 0);
      zmq::message_t reply(rpc_rep_serialized.size());
      memcpy((void*)reply.data(), rpc_rep_serialized.c_str(),
             rpc_rep_serialized.size());

      socket.send(reply);

      boost::this_thread::interruption_point();

    }
  }

  RPCRep ZmqServer::dispatch_(RPCReq rpc_req) {

    LOG(INFO) << "Extracting request string and id...";
    std::string req_str = rpc_req.request_string();
    std::string id = rpc_req.id();

    RPCRep rpc_rep;
    rpc_rep.set_id(id);

    LOG(INFO) << "Dispatch to " << req_str;

    if (req_str == "start_query") {

      std::string tag_str = rpc_req.tag();
      CHECK(!tag_str.empty());
      id = base_server_->startQuery(tag_str);
      LOG(INFO) << "Generated Query ID: " << id;
      rpc_rep.set_id(id);

    } else {

      CHECK(!id.empty());

      if (req_str == "add_trs") {

        const TrainImageUrls& urls_proto = rpc_req.train_image_urls();
        const int url_count = urls_proto.urls_size();
        CHECK(url_count > 0);

        std::vector<std::string> urls;
        for (size_t i = 0; i < url_count; ++i) {
          urls.push_back(urls_proto.urls(i));
        }

        base_server_->addTrs(id, urls);

      } else if (req_str == "train") {

        base_server_->train(id);

      } else if (req_str == "rank") {

        base_server_->rank(id);

      } else if (req_str == "get_ranking") {

        Ranking ranking = base_server_->getRanking(id);
        uint_32t page_sz = config_.server_config().page_size();

        RankedList* ranking_proto = rpc_rep.mutable_ranking();
        // extract n-th page
        uint32_t page_num = rpc_req.retrieve_page();

        size_t dset_sz = ranking.scores.rows;

        uint_32t page_count = std::ceil(static_cast<float>(dset_sz) /
                                        static_cast<float>(page_sz));
        if (page_num > page_count) {
          throw std::range_error("Tried to retrieve page outside of valid range");
        }
        ranking_proto->set_page_count(page_count);
        ranking_proto->set_page(page_num);

        size_t start_idx = page_sz*(page_num - 1);
        size_t end_idx = std::max(page_sz*page_num, dset_sz);

        for (size_t i = start_idx; i < end_idx; ++i) {
          RankedItem* ritem_proto = ranking_proto->add_rlist();
          size_t sort_idx = ranking.sort_idxs[i];
          ritem_proto->set_path(base_server_->dset_path(sort_idx));
          ritem_proto->set_score(ranking.scores[sort_idx]);
        }

      } else if (req_str == "train_rank_get_ranking") {

        // TODO : blocking version of all three above functions which
        // returns ranking directly

      } else if (req_str == "free_query") {

        base_server_->freeQuery(id);

      } else {

        rpc_rep.set_success(false);
        std::stringstream sstrm;
        sstrm << "Unrecognised function: " << req_str;
        rpc_rep.set_err_msg(sstrm.str());

        LOG(ERROR) << sstrm.str() << " ignoring...";

      }
    }

    std::string rpc_rep_str;
    google::protobuf::TextFormat::PrintToString(rpc_rep, &rpc_rep_str);
    std::cout << "Sending Protobuf:\n" << "-----------------\n"
              << rpc_rep_str << "-----------------\n" << std::endl;

    return rpc_rep;
  }

  void ZmqServer::monitor_add_trs_images_() {
    while (true) {
      QueryImageProcessedNotification notification =
        base_server_->notifier()->wait_image_processed();
      std::cout << "*******************************************************" << std::endl
                << "QUERYIMAGEPROCESSEDNOTIFICATION" << std::endl
                << "-------------------------------------------------------" << std::endl
                << "id:    " << notification.id << std::endl
                << "fname: " << notification.fname << std::endl
                << "*******************************************************" << std::endl;
    }
  }

  void ZmqServer::monitor_add_trs_complete_() {
    while (true) {
      QueryAllImagesProcessedNotification notification =
        base_server_->notifier()->wait_all_images_processed();
      std::cout << "*******************************************************" << std::endl
                << "QUERYALLIMAGESPROCESSEDNOTIFICATION" << std::endl
                << "-------------------------------------------------------" << std::endl
                << "id:    " << notification.id << std::endl
                << "*******************************************************" << std::endl;
    }
  }

}
