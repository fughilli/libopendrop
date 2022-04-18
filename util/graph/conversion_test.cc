#include "util/graph/conversion.h"

#include <tuple>

#include "googlemock/include/gmock/gmock-matchers.h"
#include "googlemock/include/gmock/gmock-more-matchers.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "util/graph/types/monotonic.h"
#include "util/graph/types/types.h"
#include "util/graph/types/unitary.h"

namespace opendrop {
namespace {

TEST(ConversionTest, CanConstructConversion) {
  std::function<std::tuple<float>(std::tuple<float>)> convert_fn =
      [](std::tuple<float> in) -> std::tuple<float> {
    return std::make_tuple(std::get<0>(in));
  };
  auto conversion = std::make_shared<Conversion>("foobar", convert_fn);
}

TEST(ConversionTest, CanInvokeConversion) {
  std::function<std::tuple<float>(std::tuple<float>)> convert_fn =
      [](std::tuple<float> in) -> std::tuple<float> {
    return std::make_tuple(std::get<0>(in) * 2);
  };
  auto conversion = std::make_shared<Conversion>("times2", convert_fn);

  EXPECT_NEAR(
      std::get<0>(
          conversion->Invoke(std::tuple<float>(123.123f)).Result<float>()),
      246.246f, 1e-6f);
}

TEST(ConversionTest, ProductionConversion) {
  std::function<std::tuple<float>()> produce_fn = []() -> std::tuple<float> {
    return std::make_tuple(123.0f);
  };
  Conversion production("production", produce_fn);

  EXPECT_NEAR(std::get<0>(production.Invoke().Result<float>()),
              123.0f, 1e-6f);
}

constexpr int kDummyTypeValue = 999;

enum ConstructionState {
  kUnconstructed,
  kConstructed,
  kDestructed,
};

ConstructionState construction_state = ConstructionState::kUnconstructed;

struct RaiiType {
  constexpr static Type kType = static_cast<Type>(kDummyTypeValue);
  RaiiType() { construction_state = ConstructionState::kConstructed; }
  RaiiType(int x) {
    construction_state = ConstructionState::kConstructed;
    member_with_default = x;
  }
  ~RaiiType() { construction_state = ConstructionState::kDestructed; }

  int member_with_default = 123;
};

TEST(ConversionTest, OutputTypeConstructorAndDestructorAreInvoked) {
  construction_state = ConstructionState::kUnconstructed;

  std::function<std::tuple<RaiiType>(std::tuple<int>)> convert_fn =
      [](std::tuple<int> in) -> std::tuple<RaiiType> {
    return std::tuple<RaiiType>();
  };

  ASSERT_EQ(construction_state, ConstructionState::kUnconstructed);
  {
    Conversion conversion("raii", convert_fn);
    EXPECT_EQ(construction_state, ConstructionState::kConstructed);
    EXPECT_EQ(std::get<0>(conversion.Result<RaiiType>()).member_with_default,
              123);
  }
  EXPECT_EQ(construction_state, ConstructionState::kDestructed);
}

// TODO: Support constructor arguments tuple for storage.
//
// Attempting to build this causes a substitution failure in std::apply().
//
// TEST(ConversionTest, OutputTypeConstructorIsInvokedWithUserArgs) {
//   construction_state = ConstructionState::kUnconstructed;
//
//   std::function<std::tuple<RaiiType>(std::tuple<int>)> convert_fn =
//       [](std::tuple<int> in) -> std::tuple<RaiiType> {
//     return std::tuple<RaiiType>();
//   };
//
//   ASSERT_EQ(construction_state, ConstructionState::kUnconstructed);
//   {
//     Conversion conversion("raii", convert_fn, std::tuple<int>(456));
//     EXPECT_EQ(construction_state, ConstructionState::kConstructed);
//     EXPECT_EQ(std::get<0>(conversion.Result<RaiiType>()).member_with_default,
//               456);
//   }
//   EXPECT_EQ(construction_state, ConstructionState::kDestructed);
// }

}  // namespace
}  // namespace opendrop
