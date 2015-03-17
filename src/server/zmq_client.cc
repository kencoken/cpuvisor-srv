#include "zmq_client.h"

#include <iostream>
#include <boost/algorithm/string/replace.hpp>

namespace cpuvisor {

  ZmqClient::ZmqClient(const visor::Config& config,
                       boost::shared_ptr<zmq::context_t> context)
    : config_(config) {

    // Prepare a ZMQ context if not supplied with one
    if (context) {
      DLOG(INFO) << "Setting context...";
      context_ = context;
    } else {
      DLOG(INFO) << "Preparing zmq context...";
      context_ = boost::shared_ptr<zmq::context_t>(new zmq::context_t(1));
    }

    // Prepare *REQ*-REP socket
    DLOG(INFO) << "Initializing REQ socket...";
    socket_ = boost::shared_ptr<zmq::socket_t>(new zmq::socket_t(*context_, ZMQ_REQ));
    std::string server_endpoint = config_.server_config().server_endpoint();
    boost::replace_all(server_endpoint, "localhost", "*");

    std::cout << "Connecting to server: " << server_endpoint << std::endl;
    socket_->connect(server_endpoint.c_str());
    std::cout << "Connected!"  << std::endl;

  }

  ZmqClient::~ZmqClient() {

  }

  void ZmqClient::addDsetImagesToIndex(const std::vector<std::string>& dset_paths) {

    // prepare request object
    RPCReq rpc_req;
    rpc_req.set_request_string("add_dset_images_to_index");
    for (size_t i = 0; i < dset_paths.size(); ++i) {
      rpc_req.add_paths(dset_paths[i]);
    }

    // serialize and send request
    std::string rpc_req_serialized;
    rpc_req.SerializeToString(&rpc_req_serialized);

    zmq::message_t request(rpc_req_serialized.size());
    memcpy((void*)request.data(), rpc_req_serialized.c_str(),
           rpc_req_serialized.size());

    socket_->send(request);

    // receive response
    zmq::message_t reply;
    socket_->recv(&reply);

    RPCRep rpc_rep;
    rpc_rep.ParseFromArray(reply.data(), reply.size());

  }

}
