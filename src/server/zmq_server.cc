#include "zmq_server.h"

#include <google/protobuf/text_format.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace cpuvisor {

  ZmqServer::ZmqServer(const cpuvisor::Config& config)
    : config_(config) {

  }

  ZmqServer::~ZmqServer() {
    // interrupt serve thread to ensure termination before auto-detaching
    if (serve_thread_) {
      serve_thread_->interrupt();
    }
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
      std::cout << "Received Protobuf:" << rpc_req_str << std::endl;

      //  Send reply back to client
      QuerySession query_ses;
      query_ses.set_id(100);

      std::string query_ses_serialized;
      query_ses.SerializeToString(&query_ses_serialized);

      zmq::message_t reply((void*)query_ses_serialized.c_str(),
                           query_ses_serialized.size(), 0);
      socket.send(reply);

      boost::this_thread::interruption_point();

    }
  }

}
