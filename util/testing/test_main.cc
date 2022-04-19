#include "absl/debugging/failure_signal_handler.h"
#include "googletest/include/gtest/gtest.h"

int main(int argc, char** argv) {
  testing::InitGoogleTest();

  // Get backtraces for abexits.
  absl::InstallFailureSignalHandler(absl::FailureSignalHandlerOptions());

  return RUN_ALL_TESTS();
}
