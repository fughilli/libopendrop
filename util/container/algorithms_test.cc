#include "util/container/algorithms.h"

#include "googlemock/include/gmock/gmock-matchers.h"
#include "googlemock/include/gmock/gmock-more-matchers.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

TEST(AlgorithmsTest, UniqueReturnsExpected) {
  EXPECT_THAT(Unique<int>({1, 2, 3}), ::testing::ElementsAre(1, 2, 3));
  EXPECT_THAT(Unique<int>({1, 2, 2, 3}), ::testing::ElementsAre(1, 2, 3));
  EXPECT_THAT(Unique<int>({1, 3, 5, 1, 7}), ::testing::ElementsAre(1, 3, 5, 7));
}

TEST(AlgorithmsTest, ModPickReturnsExpected) {
  EXPECT_EQ(ModPick<int>({1, 2, 3}, 0), 1);
  EXPECT_EQ(ModPick<int>({1, 2, 3}, 1), 2);
  EXPECT_EQ(ModPick<int>({1, 2, 3}, 2), 3);
  EXPECT_EQ(ModPick<int>({1, 2, 3}, 3), 1);
  EXPECT_EQ(ModPick<int>({1, 2, 3}, 6), 1);

  EXPECT_EQ(ModPick<int>({1, 2, 3, 4, 5}, 1), 2);
  EXPECT_EQ(ModPick<int>({1, 2, 3, 4, 5}, 3), 4);
  EXPECT_EQ(ModPick<int>({1, 2, 3, 4, 5}, 2), 3);
  EXPECT_EQ(ModPick<int>({1, 2, 3, 4, 5}, 5), 1);
  EXPECT_EQ(ModPick<int>({1, 2, 3, 4, 5}, 10), 1);
}

TEST(AlgorithmsTest, WeightedPickReturnsExpected) {
  {
    auto picker = [](float r) -> int {
      return WeightedPick<int>({1, 2, 3, 4}, {0.1f, 0.2f, 0.4f, 0.3f}, r);
    };
    EXPECT_EQ(picker(-2), 1);
    EXPECT_EQ(picker(0), 1);

    EXPECT_EQ(picker(0.05), 1);
    EXPECT_EQ(picker(0.15), 2);
    EXPECT_EQ(picker(0.25), 2);
    EXPECT_EQ(picker(0.35), 3);
    EXPECT_EQ(picker(0.45), 3);
    EXPECT_EQ(picker(0.55), 3);
    EXPECT_EQ(picker(0.65), 3);
    EXPECT_EQ(picker(0.75), 4);
    EXPECT_EQ(picker(0.85), 4);
    EXPECT_EQ(picker(0.95), 4);

    EXPECT_EQ(picker(1), 4);
    EXPECT_EQ(picker(5), 4);
  }
}

TEST(AlgorithmsTest, RandomIndexOfReturnsExpected) {
  EXPECT_EQ(RandomIndexOf<int>({1, 2, 3}, 1), 0);
  EXPECT_EQ(RandomIndexOf<int>({1, 2, 3}, 2), 1);
  EXPECT_EQ(RandomIndexOf<int>({1, 2, 3}, 3), 2);
}

TEST(AlgorithmsTest, RandomIndexDiesOnNotFound) {
  EXPECT_DEATH({ RandomIndexOf<int>({1, 2, 3}, 5); }, "not found");
}

TEST(AlgorithmsTest, RandomIndexHasExpectedStatistics) {
  constexpr int kNumTrials = 1000;

  std::vector<size_t> results{};
  auto func = [](int x) -> size_t {
    return RandomIndexOf<int>({1, 2, 3, 1, 2, 3}, x);
  };

  for (int i = 0; i < kNumTrials; ++i) {
    results.push_back(func(1));
  }
  EXPECT_NEAR(std::count(results.begin(), results.end(), 0), 500, 50);
  EXPECT_EQ(std::count(results.begin(), results.end(), 1), 0);
  EXPECT_EQ(std::count(results.begin(), results.end(), 2), 0);
  EXPECT_NEAR(std::count(results.begin(), results.end(), 3), 500, 50);
  EXPECT_EQ(std::count(results.begin(), results.end(), 4), 0);
  EXPECT_EQ(std::count(results.begin(), results.end(), 5), 0);

  results.clear();
  for (int i = 0; i < kNumTrials; ++i) {
    results.push_back(func(3));
  }
  EXPECT_EQ(std::count(results.begin(), results.end(), 0), 0);
  EXPECT_EQ(std::count(results.begin(), results.end(), 1), 0);
  EXPECT_NEAR(std::count(results.begin(), results.end(), 2), 500, 50);
  EXPECT_EQ(std::count(results.begin(), results.end(), 3), 0);
  EXPECT_EQ(std::count(results.begin(), results.end(), 4), 0);
  EXPECT_NEAR(std::count(results.begin(), results.end(), 5), 500, 50);
}

}  // namespace
}  // namespace opendrop
