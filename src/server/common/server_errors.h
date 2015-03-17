////////////////////////////////////////////////////////////////////////////
//    File:        server_errors.h
//    Author:      Ken Chatfield
//    Description: Server error exceptions
////////////////////////////////////////////////////////////////////////////

#ifndef VISOR_SERVER_ERRORS_H_
#define VISOR_SERVER_ERRORS_H_

#include <stdexcept>

namespace visor {

  // exceptions --------------------------

  class InvalidRequestError: public std::runtime_error {
  public:
    InvalidRequestError(std::string const& msg): std::runtime_error(msg) { }
  };

}

#endif
