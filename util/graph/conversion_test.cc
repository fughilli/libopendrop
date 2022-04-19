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

//
// Tests covering ConversionToOpaqueFunction().
//

TEST(TupleTest, OpaqueTupleFunctionInvocation) {
  auto input_tuple = OpaqueTuple::ConstructFromTypes<float, float>();
  auto output_tuple = OpaqueTuple::ConstructFromTypes<float, int>();

  std::function<std::tuple<float, int>(std::tuple<float&, float&>)> convert_fn =
      [](std::tuple<float&, float&> in) -> std::tuple<float, int> {
    auto& [a, b] = in;
    return std::make_tuple(a + b, static_cast<int>(a * b));
  };
  auto opaque_fn = ConversionToOpaqueFunction(convert_fn);

  input_tuple.AssignFrom(std::make_tuple(10.0f, 3.5f));
  opaque_fn(input_tuple, output_tuple);

  EXPECT_EQ(output_tuple.Get<float>(0), 13.5f);
  EXPECT_EQ(output_tuple.Get<int>(1), 35);
}

TEST(TupleTest, OpaqueTupleFactoryFunctionInvocation) {
  auto input_tuple_factory = OpaqueTupleFactory::FromTypes<float, float>();
  auto output_tuple_factory = OpaqueTupleFactory::FromTypes<float, int>();

  std::function<std::tuple<float, int>(std::tuple<float&, float&>)> convert_fn =
      [](std::tuple<float&, float&> in) -> std::tuple<float, int> {
    auto& [a, b] = in;
    return std::make_tuple(a + b, static_cast<int>(a * b));
  };
  auto opaque_fn = ConversionToOpaqueFunction(convert_fn);

  auto input_tuple = input_tuple_factory.Construct();
  auto output_tuple = output_tuple_factory.Construct();

  input_tuple.AssignFrom(std::make_tuple(10.0f, 3.5f));
  opaque_fn(input_tuple, output_tuple);

  EXPECT_EQ(output_tuple.Get<float>(0), 13.5f);
  EXPECT_EQ(output_tuple.Get<int>(1), 35);
}

TEST(TupleTest, OpaqueTupleAliasingFunctionInvocations) {
  // Compute a function graph:
  //
  // A := float(float, float)  // Add
  // B := int(float, float)    // Multiply, convert to `int`
  // C := float(int, float)    // Subtract
  //
  //        +---------------------------+
  //        | input_tuple: float, float |
  //        +---------------------------+
  //            |                    |
  //           [B]                  [C]
  //            |                    |
  //            V                    V
  // +--------------------+  +------------------+
  // | mid_tuple_1: float |  | mid_tuple_2: int |
  // +--------------------+  +------------------+
  //              |               |
  //              |               |
  //              V               V
  //      +---------------------------------+
  //      | mid_tuple_combined : int, float |
  //      +---------------------------------+
  //                      |
  //                     [C]
  //                      |
  //                      V
  //           +---------------------+
  //           | output_tuple: float |
  //           +---------------------+
  //
  // `mid_tuple_1` and `mid_tuple_2` own their storage.
  //
  // `mid_tuple_combined` is an aliasing OpaqueTuple over both of them.
  auto input_tuple = OpaqueTuple::ConstructFromTypes<float, float>();
  auto mid_tuple_1 = OpaqueTuple::ConstructFromTypes<float>();
  auto mid_tuple_2 = OpaqueTuple::ConstructFromTypes<int>();
  auto mid_tuple_combined = OpaqueTuple::EmptyFromTypes<int, float>();
  auto output_tuple = OpaqueTuple::ConstructFromTypes<float>();

  mid_tuple_combined.Alias(1, mid_tuple_1, 0);
  mid_tuple_combined.Alias(0, mid_tuple_2, 0);

  std::function<std::tuple<float>(std::tuple<float&, float&>)> a_fn =
      [](std::tuple<float&, float&> in) -> std::tuple<float> {
    auto& [a, b] = in;
    return std::make_tuple(a + b);
  };
  std::function<std::tuple<int>(std::tuple<float&, float&>)> b_fn =
      [](std::tuple<float&, float&> in) -> std::tuple<int> {
    auto& [a, b] = in;
    return std::make_tuple(static_cast<int>(a * b));
  };
  std::function<std::tuple<float>(std::tuple<int&, float&>)> c_fn =
      [](std::tuple<int&, float&> in) -> std::tuple<float> {
    auto& [a, b] = in;
    return std::make_tuple(a - b);
  };

  auto opaque_a_fn = ConversionToOpaqueFunction(a_fn);
  auto opaque_b_fn = ConversionToOpaqueFunction(b_fn);
  auto opaque_c_fn = ConversionToOpaqueFunction(c_fn);

  input_tuple.AssignFrom(std::make_tuple(10.0f, 3.5f));

  opaque_a_fn(input_tuple, mid_tuple_1);
  opaque_b_fn(input_tuple, mid_tuple_2);
  opaque_c_fn(mid_tuple_combined, output_tuple);

  EXPECT_EQ(output_tuple.Get<float>(0), 21.5f);
}

//
// Tests covering `Conversion`.
//

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
