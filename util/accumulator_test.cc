#include "libopendrop/util/accumulator.h"

#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

TEST(AccumulatorTest, AccumulatorDefaultInitializerIsZero) {
  Accumulator<float> accumulator;
  EXPECT_EQ(accumulator.value(), 0);
}

TEST(AccumulatorTest, AccumulatorUpdateIncrementsValue) {
  Accumulator<float> accumulator;
  ASSERT_EQ(accumulator.value(), 0);
  EXPECT_EQ(accumulator.Update(5.0f), 5.0f);
  EXPECT_EQ(accumulator.Update(3.2f), 8.2f);
}

TEST(AccumulatorTest, AccumulatorCanUpdateWithPlusEquals) {
  Accumulator<float> accumulator;
  ASSERT_EQ(accumulator.value(), 0);
  EXPECT_EQ(accumulator += 5.0f, 5.0f);
  EXPECT_EQ(accumulator += 3.2f, 8.2f);
}

TEST(AccumulatorTest, AccumulatorBuilderCanInitializeWithValue) {
  auto accumulator = Accumulator<int>().SetValue(2);
  EXPECT_EQ(accumulator.value(), 2);
}

TEST(AccumulatorTest, AccumulatorCanImplicitlyConvertToT) {
  auto accumulator = Accumulator<float>().SetValue(2);
  EXPECT_EQ(accumulator, 2.0f);
}

TEST(AccumulatorTest, AccumulatorBuilderCanInitializeWithPeriod) {
  auto accumulator = Accumulator<int>().SetPeriod(12);
  ASSERT_EQ(accumulator.value(), 0);

  EXPECT_EQ(accumulator.Update(6), 6);
  EXPECT_EQ(accumulator.Update(7), 1);
  EXPECT_EQ(accumulator.Update(121), 2);
}

TEST(AccumulatorTest, AccumulatorLastStepReturnsLastStep) {
  Accumulator<int> accumulator;
  ASSERT_EQ(accumulator.last_step(), 0);

  accumulator.Update(2);
  EXPECT_EQ(accumulator.last_step(), 2);
  accumulator.Update(5);
  EXPECT_EQ(accumulator.last_step(), 5);
  accumulator.Update(-3);
  EXPECT_EQ(accumulator.last_step(), -3);
}

TEST(AccumulatorTest,
     AccumulatorInterpolateLastStepReturnsCorrectInterpolator) {
  auto accumulator = Accumulator<int>().SetValue(10);
  accumulator.Update(5);

  auto interpolator = accumulator.InterpolateLastStep(2);
  EXPECT_EQ(interpolator.begin_value(), 10);
  EXPECT_EQ(interpolator.end_value(), 15);
  EXPECT_EQ(interpolator.step(), 2);
}

}  // namespace
}  // namespace opendrop
