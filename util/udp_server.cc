#include "util/udp_server.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include "util/logging.h"

namespace util {

bool UdpServer::Start() {
  socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd_ < 0) {
    return false;
  }

  struct sockaddr_in servaddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port_),
      .sin_addr =
          {
              .s_addr = INADDR_ANY,
          },
  };

  if (bind(socket_fd_, reinterpret_cast<const struct sockaddr*>(&servaddr),
           sizeof(servaddr)) < 0) {
    return false;
  }

  return true;
}

bool UdpServer::Read(std::vector<uint8_t>* buffer) {
  int actual_bytes = recv(socket_fd_, receive_buffer_.data(),
                          receive_buffer_.size(), MSG_DONTWAIT);
  if (actual_bytes <= 0) {
    return false;
  }
  buffer->resize(actual_bytes);
  std::copy(receive_buffer_.data(), &receive_buffer_[actual_bytes],
            buffer->begin());
  return true;
}

}  // namespace util
