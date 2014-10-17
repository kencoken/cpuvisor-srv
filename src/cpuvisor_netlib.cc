#include <iostream>
#include <string>

#include <glog/logging.h>

#include <boost/network/protocol/http/client.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/network/protocol/http/response.hpp>
#include <boost/network/protocol/http/client/connection/connection_delegate_factory.hpp>
#include <boost/network/protocol/http/traits/delegate_factory.hpp>
#include <boost/network/protocol/http/client/connection/async_normal.hpp>

#include <unistd.h>

using namespace boost::network;
using namespace boost::network::http;

// typedef boost::function<void(boost::iterator_range<char const *> const &,
//                              boost::system::error_code const &)> body_callback_function_type;
// typedef boost::iterator_range<char const *> rValue;

// std::string res;

// void callback(boost::iterator_range<char const *> const & a,
//               boost::system::error_code const & b) {
//   std::cout << "In callback with " << a.size() << "characters" << std::endl;
//   for(rValue::difference_type i = 0; i < a.size(); ++i) {
//     res += a[i];
//   }
// }

struct body_handler {
  explicit body_handler() { body = ""; }

  BOOST_NETWORK_HTTP_BODY_CALLBACK(operator(), range, error) {
    // in here, range is the Boost.Range iterator_range, and error is
    // the Boost.System error code.
    if (!error) {
      std::cout << body.size() << ",..." << std::endl;
      body.append(boost::begin(range), boost::end(range));
      std::cout << "...," << body.size() << std::endl;
    }
  }

  std::string body;
};

int main(int argc, char* argv[]) {

  google::InstallFailureSignalHandler();

  client::options options;
  options.follow_redirects(true);

  http::client client(options);
  http::client::request request("http://upload.wikimedia.org/wikipedia/commons/5/58/Sunset_2007-1.jpg");
  http::client::response response;

  std::cout << "Issuing request" << std::endl;
  response = client.get(request, body_handler());

  std::cout << "Waiting for response..." << std::endl;
  while (!ready(response));

  usleep(100);

  std::cout << "OK!" << std::endl;
  //std::cout << res << std::endl;

  return 0;
}
