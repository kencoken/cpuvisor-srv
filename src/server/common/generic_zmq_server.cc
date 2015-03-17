#include "generic_zmq_server.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <google/protobuf/text_format.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "server/common/server_errors.h"

namespace visor {

  GenericZmqServer::GenericZmqServer(const visor::Config& config)
    : config_(config) { }

  GenericZmqServer::~GenericZmqServer() {
    // interrupt serve thread to ensure termination before auto-detaching
    if (serve_thread_) serve_thread_->interrupt();
  }

  void GenericZmqServer::serve(const bool blocking) {
    if (!serve_thread_) {
      serve_thread_.reset(new boost::thread(&GenericZmqServer::serve_, this));
    }
    if (blocking) {
      serve_thread_->join();
    }
  }

  // -----------------------------------------------------------------------------

  void GenericZmqServer::serve_() {
    // Prepare our context
    zmq::context_t context(1);

    // Prepare REQ-*REP* socket
    zmq::socket_t socket(context, ZMQ_REP);
    std::string server_endpoint = config_.server_config().server_endpoint();
    boost::replace_all(server_endpoint, "localhost", "*");
    socket.bind(server_endpoint.c_str());

    // Prepare *PUB*-SUB socket
    boost::shared_ptr<zmq::socket_t> notify_socket(new zmq::socket_t(context, ZMQ_PUB));
    std::string notify_endpoint = config_.server_config().notify_endpoint();
    boost::replace_all(notify_endpoint, "localhost", "*");
    notify_socket->bind(notify_endpoint.c_str());

    notify_socket_ = notify_socket; // store when ready for use in notify threads

    std::cout << "ZMQ server started..." << std::endl;

    while (true) {
      zmq::message_t request;

      //  Wait for next request from client
      socket.recv(&request);
      RPCReq rpc_req;
      RPCRep rpc_rep;
      if (rpc_req.ParseFromArray(request.data(), request.size())) {

        #ifndef NDEBUG
        {
          std::string rpc_req_str;
          google::protobuf::TextFormat::PrintToString(rpc_req, &rpc_req_str);
          DLOG(INFO) << "Received Protobuf:\n" << "->->->->->->->->\n"
                     << rpc_req_str << "->->->->->->->->\n";
        }
        #endif
        std::cout << "**********************************\n"
                  << "Received request: " << rpc_req.request_string()
                  << ", query_id: " << rpc_req.id() << ", tag: " << rpc_req.tag() << std::endl
                  << "**********************************\n";

        rpc_rep = dispatch_(rpc_req);

      } else {

        rpc_rep.set_success(false);
        rpc_rep.set_err_msg("Could not parse request object");

        LOG(ERROR) << "Could not parse request object - ignoring...";


      }

      //  Send reply back to client
      std::string rpc_rep_serialized;
      rpc_rep.SerializeToString(&rpc_rep_serialized);

      #ifndef NDEBUG
      {
        std::string rpc_rep_str;
        google::protobuf::TextFormat::PrintToString(rpc_rep, &rpc_rep_str);

        DLOG(INFO) << "Sending Protobuf:\n" << "<-<-<-<-<-<-<-<-\n"
                   << "**** Formatted:\n"
                   << rpc_rep_str
                   << "**** Raw:\n"
                   << rpc_rep_serialized << std::endl
                   << "<-<-<-<-<-<-<-<-\n";
      }
      #endif

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

  RPCRep GenericZmqServer::dispatch_(const RPCReq& rpc_req) {

    LOG(INFO) << "Extracting request string and id...";
    std::string req_str = rpc_req.request_string();
    std::string id = rpc_req.id();

    RPCRep rpc_rep;
    rpc_rep.set_id(id);
    // not sure why, but need to set the success flag explicitly for
    // some protobuf installations, despite the fact that it has a
    // default value of true
    rpc_rep.set_success(true);

    LOG(INFO) << "Dispatch to " << req_str;

    try {

      if (!dispatch_handler_(rpc_req, rpc_rep)) {

        rpc_rep.set_success(false);
        std::stringstream sstrm;
        sstrm << "Unrecognised function: " << req_str;
        rpc_rep.set_err_msg(sstrm.str());

        LOG(ERROR) << sstrm.str() << " ignoring...";

      }

    } catch (InvalidRequestError& e) {

      rpc_rep.set_success(false);
      std::stringstream sstrm;
      sstrm << "Request was invalid: " << e.what();
      rpc_rep.set_err_msg(sstrm.str());

      LOG(ERROR) << "Request error occurred: " << sstrm.str();

    }

    return rpc_rep;
  }

  void GenericZmqServer::getRankingPage_(const Ranking& ranking,
                                         const RPCReq& rpc_req, RPCRep* rpc_rep) {

    uint32_t page_sz = config_.server_config().page_size();

    RankedList* ranking_proto = rpc_rep->mutable_ranking();
    // extract n-th page
    uint32_t page_num = rpc_req.retrieve_page();

    // construct ranking proto object
    getRankingProto_(ranking, ranking_proto, page_sz, page_num);

  }

  void GenericZmqServer::getRankingProto_(const Ranking& ranking,
                                          RankedList* ranking_proto,
                                          const size_t page_sz,
                                          const size_t page_num) {

    uint32_t dset_sz = ranking.scores.rows;
    CHECK_GE(dset_sz, ranking.scores.cols);
    CHECK_EQ(ranking.scores.cols, 1);

    uint32_t page_count;
    size_t actual_page_sz = page_sz;
    if (page_sz < 1) {
      page_count = 1;
    } else {
      page_count = std::ceil(static_cast<float>(dset_sz) /
                             static_cast<float>(page_sz));
      if (page_sz > dset_sz) actual_page_sz = dset_sz;
    }

    DLOG(INFO) << "Page size is: " << actual_page_sz;
    DLOG(INFO) << "Page num is: " << page_num;
    DLOG(INFO) << "Page count is: " << page_count;

    if (page_num > page_count) {
      throw std::range_error("Tried to retrieve page outside of valid range");
    }
    ranking_proto->set_page_count(page_count);
    ranking_proto->set_page(page_num);

    size_t start_idx = actual_page_sz*(page_num - 1);
    size_t end_idx = std::min(actual_page_sz*page_num, static_cast<size_t>(dset_sz));

    CHECK_EQ(ranking.sort_idxs.type(), CV_32S);
    CHECK_EQ(ranking.scores.type(), CV_32F);
    uint32_t* sort_idxs_ptr = (uint32_t*)ranking.sort_idxs.data;
    float* scores_ptr = (float*)ranking.scores.data;

    for (size_t i = 0; i < 10; ++i) {
      DLOG(INFO) << i+1 << ": " << base_server_->dset_path(sort_idxs_ptr[i])
                 << " (" << scores_ptr[sort_idxs_ptr[i]] << ")";
    }

    DLOG(INFO) << "Page start idx is: " << start_idx;
    DLOG(INFO) << "Page end idx is: " << end_idx;

    for (size_t i = start_idx; i < end_idx; ++i) {
      RankedItem* ritem_proto = ranking_proto->add_rlist();
      uint32_t sort_idx = sort_idxs_ptr[i];
      ritem_proto->set_path(base_server_->dset_path(sort_idx));
      ritem_proto->set_score(scores_ptr[sort_idx]);
    }

  }

  void GenericZmqServer::notify_(const VisorNotification& notify_proto) {
    if (notify_socket_) {
      std::string notify_proto_serialized;
      notify_proto.SerializeToString(&notify_proto_serialized);

      zmq::message_t notify_msg(notify_proto_serialized.size());
      memcpy((void*)notify_msg.data(), notify_proto_serialized.c_str(),
             notify_proto_serialized.size());

      notify_socket_->send(notify_msg);
    }
  }

}
