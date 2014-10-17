#include "zmq_server.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace cpuvisor {

  ZmqServer::ZmqServer(const cpuvisor::Config& config) {

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
    socket.bind ("tcp://*:5555");

    std::cout << "ZMQ server started..." << std::endl;

    while (true) {
      zmq::message_t request;

      //  Wait for next request from client
      socket.recv(&request);
      std::cout << "Received Hello" << std::endl;

      //  Send reply back to client
      std::string reply_str = "World";
      zmq::message_t reply((void*)reply_str.c_str(), reply_str.size(), 0);
      socket.send(reply);

      boost::this_thread::interruption_point();

    }
  }

}
