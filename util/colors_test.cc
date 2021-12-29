#include "util/colors.h"

#include <glm/geometric.hpp>

#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

constexpr float kEpsilon = 1e-6f;

struct TestValues {
  glm::vec3 hsv_input;
  glm::vec3 rgb_expected_output;
};

class ColorsParameterizedTest : public ::testing::TestWithParam<TestValues> {};
TEST_P(ColorsParameterizedTest, HsvToRgbProducesCorrectValues) {
  TestValues values = GetParam();

  auto rgb = HsvToRgb(values.hsv_input);
  EXPECT_LE(glm::length(rgb - values.rgb_expected_output), kEpsilon);
}

INSTANTIATE_TEST_SUITE_P(
    CorrectValuesSequence, ColorsParameterizedTest,
    ::testing::Values(TestValues{{0, 1, 1}, {1, 0, 0}},
                      TestValues{{1.0f / 3.0f, 1, 1}, {0, 1, 0}},
                      TestValues{{2.0f / 3.0f, 1, 1}, {0, 0, 1}},
                      TestValues{{0, 0, 0}, {0, 0, 0}},
                      TestValues{{10000000, 1, 1}, {1, 0, 0}}));

}  // namespace
}  // namespace opendrop
