#ifndef DEBUG_PROTO_PORT_H_
#define DEBUG_PROTO_PORT_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "util/udp_server.h"

template <typename P>
class ProtoPort {
 public:
  ProtoPort(int port) : udp_server_(port) { udp_server_.Start(); }

  bool Read(P* proto) {
    std::vector<uint8_t> buffer{};

    if (!udp_server_.Read(&buffer)) return false;

    std::string data_to_parse = {reinterpret_cast<const char*>(buffer.data()),
                                 buffer.size()};
    proto->ParseFromString(data_to_parse);
    return true;
  }

 private:
  util::UdpServer udp_server_;
};

#endif  // DEBUG_PROTO_PORT_H_
