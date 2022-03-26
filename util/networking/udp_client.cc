#include "util/networking/udp_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace util {

bool UdpClient::Start() {
  socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd_ < 0) {
    return false;
  }

  struct sockaddr_in cliaddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port_),
      .sin_addr =
          {
              .s_addr = inet_addr(server_.c_str()),
          },
  };

  if (connect(socket_fd_, reinterpret_cast<const struct sockaddr*>(&cliaddr),
              sizeof(cliaddr)) < 0) {
    return false;
  }

  return true;
}

bool UdpClient::Read(std::vector<uint8_t>* buffer) {
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

bool UdpClient::Write(const std::vector<uint8_t>& buffer) {
  return send(socket_fd_, buffer.data(), buffer.size(), MSG_DONTWAIT) ==
         buffer.size();
}

}  // namespace util
