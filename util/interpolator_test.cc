#include "libopendrop/util/interpolator.h"

#include <vector>

#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

TEST(InterpolatorTest, InterpolatorDiesOnIncorrectStepSign) {
  ASSERT_DEATH({ Interpolator<int>(2, 5, -1); }, "sign");
  ASSERT_DEATH({ Interpolator<int>(7, 1, 2); }, "sign");
}

TEST(InterpolatorTest, InterpolatorInitializerPopulatesCorrectValues) {
  Interpolator<int> interpolator(1, 2, 3);
  EXPECT_EQ(interpolator.begin_value(), 1);
  EXPECT_EQ(interpolator.end_value(), 2);
  EXPECT_EQ(interpolator.step(), 3);
}

TEST(InterpolatorTest, InterpolatorIteratorsDereferenceToCorrectValues) {
  Interpolator<int> interpolator(1, 2, 3);
  EXPECT_EQ(*(interpolator.begin()), 1);
  EXPECT_EQ(*(interpolator.end()), 2);
}

TEST(InterpolatorTest, InterpolatorIteratorIncrementWorksCorrectly) {
  Interpolator<int> interpolator(4, 12, 3);
  auto iterator = interpolator.begin();
  EXPECT_EQ(*iterator, 4);
  iterator++;
  EXPECT_EQ(*iterator, 7);
  iterator++;
  EXPECT_EQ(*iterator, 10);
  iterator++;
  EXPECT_EQ(*iterator, 12);
  iterator++;
  EXPECT_EQ(iterator, interpolator.end());
}

TEST(InterpolatorTest,
     InterpolatorIteratorIncrementAndDereferenceWorksCorrectly) {
  Interpolator<int> interpolator(4, 12, 3);
  auto iterator = interpolator.begin();
  EXPECT_EQ(*iterator++, 4);
  EXPECT_EQ(*iterator++, 7);
  EXPECT_EQ(*iterator++, 10);
  EXPECT_EQ(*iterator++, 12);
  EXPECT_EQ(iterator, interpolator.end());
}

TEST(InterpolatorTest,
     InterpolatorIteratorPreIncrementAndDereferenceWorksCorrectly) {
  Interpolator<int> interpolator(4, 12, 3);
  auto iterator = interpolator.begin();
  EXPECT_EQ(*++iterator, 7);
  EXPECT_EQ(*++iterator, 10);
  EXPECT_EQ(*++iterator, 12);
  ++iterator;
  EXPECT_EQ(iterator, interpolator.end());
}

struct TestValues {
  std::vector<int> expected_values;
  std::tuple<int, int, int> interpolator_params;
};

class InterpolatorParameterizedTest
    : public ::testing::TestWithParam<TestValues> {};
TEST_P(InterpolatorParameterizedTest, InterpolatorProducesCorrectValues) {
  TestValues values = GetParam();

  auto params = values.interpolator_params;
  Interpolator<int> interpolator(std::get<0>(params), std::get<1>(params),
                                 std::get<2>(params));

  int index = 0;
  for (auto value : interpolator) {
    EXPECT_EQ(value, values.expected_values[index++]);
  }

  EXPECT_EQ(index, values.expected_values.size());
}

INSTANTIATE_TEST_SUITE_P(
    CorrectValuesSequence, InterpolatorParameterizedTest,
    ::testing::Values(TestValues{{1, 3, 5, 7, 8}, {1, 8, 2}},
                      TestValues{{1, 2}, {1, 2, 100}},
                      TestValues{{5, 4, 3, 2}, {5, 2, -1}}));

}  // namespace
}  // namespace opendrop
