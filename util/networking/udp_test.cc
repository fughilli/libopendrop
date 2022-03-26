#include <memory>
#include <utility>
#include <vector>

#include "googlemock/include/gmock/gmock-matchers.h"
#include "googletest/include/gtest/gtest.h"
#include "util/networking/udp_client.h"
#include "util/networking/udp_server.h"

namespace util {
namespace {

class UdpTest : public ::testing::Test {
 public:
  UdpTest() {
    server_ = std::make_shared<UdpServer>(1234);
    client_ = std::make_shared<UdpClient>("127.0.0.1", 1234);
  }
  ~UdpTest() override {}

 protected:
  std::shared_ptr<UdpServer> server_;
  std::shared_ptr<UdpClient> client_;
};

TEST_F(UdpTest, ClientSendIsReceivedAtServer) {
  ASSERT_TRUE(server_->Start());
  ASSERT_TRUE(client_->Start());

  ASSERT_TRUE(client_->Write({1, 2, 3, 4, 5}));
  std::vector<uint8_t> received;
  ASSERT_TRUE(server_->Read(&received));
  EXPECT_THAT(received, ::testing::ElementsAre(1, 2, 3, 4, 5));
}

}  // namespace
}  // namespace util
