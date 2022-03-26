#include "util/signal/filter.h"

#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

TEST(FilterTest, ConstructFirFilter) {
  FirFilter fir_filter({0.1f, 0.2f, 0.3f, 0.4f, 0.5f});
}

TEST(FilterTest, ConstructIirFilter) {
  FirFilter fir_filter({0.1f, 0.2f, 0.3f, 0.4f, 0.5f});
}

}  // namespace
}  // namespace opendrop
