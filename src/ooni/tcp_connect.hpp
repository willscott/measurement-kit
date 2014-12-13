#ifndef LIBIGHT_OONI_TCP_CONNECT_HPP
# define LIBIGHT_OONI_TCP_CONNECT_HPP

#include "ooni/tcp_test.hpp"
#include <sys/stat.h>

using namespace ight::ooni::tcp_test;

namespace ight {
namespace ooni {
namespace tcp_connect {

class InputFileDoesNotExist : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class InputFileRequired : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class TCPConnect : public TCPTest {
    using TCPTest::TCPTest;

    TCPClient client;

public:
    TCPConnect(std::string input_filepath_, ight::common::Settings options_) : 
      TCPTest(input_filepath_, options_) {
        test_name = "dns_injection";
        test_version = "0.0.1";

        if (input_filepath_ == "") {
          throw InputFileRequired("An input file is required!");
        }

        struct stat buffer;   
        if (stat(input_filepath_.c_str(), &buffer) != 0) {
          throw InputFileDoesNotExist(input_filepath_+" does not exist");
        }
    };

    void main(std::string input, ight::common::Settings options,
              std::function<void(ReportEntry)>&& cb);
};

}}}

#endif  // LIBIGHT_OONI_TCP_CONNECT_HPP
