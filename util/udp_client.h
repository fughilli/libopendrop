#ifndef UTIL_UDP_CLIENT_H_
#define UTIL_UDP_CLIENT_H_

#include <stdint.h>

#include <string>
#include <vector>

namespace utils {

class UdpClient {
 public:
  UdpClient(std::string server, int port)
      : server_(server), port_(port), receive_buffer_(65536) {}

  bool Start();

  bool Read(std::vector<uint8_t>* buffer);
  bool Write(const std::vector<uint8_t>& buffer);

 private:
  std::string server_;
  int port_;
  int socket_fd_;

  std::vector<uint8_t> receive_buffer_;
  std::vector<uint8_t> transmit_buffer_;
};

}  // namespace utils

#endif  // UTIL_UDP_CLIENT_H_
