#include "libopendrop/util/math.h"

#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

TEST(MathTest, AlmostEqualTrueForSameValue) {
  EXPECT_TRUE(AlmostEqual<int>(0,0));
  EXPECT_TRUE(AlmostEqual<float>(0,0));
  EXPECT_TRUE(AlmostEqual<double>(11.123,11.123));
  EXPECT_TRUE(AlmostEqual<double>(5,5));
  EXPECT_TRUE(AlmostEqual<double>(-22.345,-22.345));
}

TEST(MathTest, AlmostEqualTrueForSlightlyDifferentValues) {
  EXPECT_TRUE(AlmostEqual(1.0f,1.0f + 1e-10f));
  EXPECT_TRUE(AlmostEqual(1.0f,1.0f + 1e-8f));
  EXPECT_TRUE(AlmostEqual(1.0f,1.0f + 1e-6f));
  EXPECT_FALSE(AlmostEqual(1.0f,1.0f + 1e-5f));
}

TEST(MathTest, SafeDivideReturnsNumeratorForSmallDenominator) {
  EXPECT_NEAR(SafeDivide(5.0f, 2.0f), 2.5f, 1e-6f);
  EXPECT_NEAR(SafeDivide(5.0f, 1e-8f), 5.0f, 1e-6f);
}

}  // namespace
}  // namespace opendrop
