#include "util/math.h"

#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

TEST(MathTest, AlmostEqualTrueForSameValue) {
  EXPECT_TRUE(AlmostEqual<int>(0, 0));
  EXPECT_TRUE(AlmostEqual<float>(0, 0));
  EXPECT_TRUE(AlmostEqual<double>(11.123, 11.123));
  EXPECT_TRUE(AlmostEqual<double>(5, 5));
  EXPECT_TRUE(AlmostEqual<double>(-22.345, -22.345));
}

TEST(MathTest, AlmostEqualTrueForSlightlyDifferentValues) {
  EXPECT_TRUE(AlmostEqual(1.0f, 1.0f + 1e-10f));
  EXPECT_TRUE(AlmostEqual(1.0f, 1.0f + 1e-8f));
  EXPECT_TRUE(AlmostEqual(1.0f, 1.0f + 1e-6f));
  EXPECT_FALSE(AlmostEqual(1.0f, 1.0f + 1e-5f));
}

TEST(MathTest, SafeDivideReturnsNumeratorForSmallDenominator) {
  EXPECT_NEAR(SafeDivide(5.0f, 2.0f), 2.5f, kEpsilon);
  EXPECT_NEAR(SafeDivide(5.0f, 1e-8f), 5.0f, kEpsilon);
}

TEST(MathTest, LogLinearIsLinearPlusConstantAtNEqualsOne) {
  std::array<double, 5> test_values = {0.00001, 0.001, 0.1, 10, 1000};
  double constant = LogLinear(test_values[0], 1.0) - test_values[0];
  for (auto value : test_values) {
    EXPECT_NEAR(LogLinear(value, 1.0), value + constant, kEpsilon);
  }
}

TEST(MathTest, LogLinearIsLogAtNEqualsZero) {
  std::array<double, 5> test_values = {0.00001, 0.001, 0.1, 10, 1000};
  for (auto value : test_values) {
    EXPECT_NEAR(LogLinear(value, 0.0), std::log(value), kEpsilon);
  }
}

TEST(MathTest, MapValueProducesExpectedValue) {
  EXPECT_NEAR(MapValue<float>(0.5, 0.0, 1.0, 0.0, 10.0), 5.0, kEpsilon);
  EXPECT_NEAR((MapValue<float, false>(-0.5, 0.0, 1.0, 0.0, 10.0)), -5.0,
              kEpsilon);
  EXPECT_NEAR((MapValue<float, true>(-0.5, 0.0, 1.0, 0.0, 10.0)), 0.0,
              kEpsilon);
  EXPECT_NEAR((MapValue<float, false>(4, 2, 6, -10, -20)), -15, kEpsilon);
}

TEST(MathTest, WrapToRangeProducesExpectedValue) {
  EXPECT_NEAR(WrapToRange<float>(-0.4f, 0.0f, 1.0f), 0.6f, kEpsilon);
  EXPECT_NEAR(WrapToRange<float>(0.0f, 0.0f, 1.0f), 0.0f, kEpsilon);
  EXPECT_NEAR(WrapToRange<float>(0.5f, 0.0f, 1.0f), 0.5f, kEpsilon);
  EXPECT_NEAR(WrapToRange<float>(0.99f, 0.0f, 1.0f), 0.99f, kEpsilon);
  EXPECT_NEAR(WrapToRange<float>(1.0f, 0.0f, 1.0f), 0.0f, kEpsilon);
  EXPECT_NEAR(WrapToRange<float>(1.2f, 0.0f, 1.0f), 0.2f, kEpsilon);

  EXPECT_NEAR(WrapToRange<float>(-4.0f, -2.0f, 3.0f), 1.0f, kEpsilon);

  EXPECT_EQ(WrapToRange<int>(-4, 0, 10), 6);
  EXPECT_EQ(WrapToRange<int>(0, 0, 10), 0);
  EXPECT_EQ(WrapToRange<int>(5, 0, 10), 5);
  EXPECT_EQ(WrapToRange<int>(9, 0, 10), 9);
  EXPECT_EQ(WrapToRange<int>(10, 0, 10), 0);
  EXPECT_EQ(WrapToRange<int>(12, 0, 10), 2);

  EXPECT_EQ(WrapToRange<int>(-4, -2, 3), 1);
}

}  // namespace
}  // namespace opendrop
