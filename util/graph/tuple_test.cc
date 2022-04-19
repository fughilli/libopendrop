#include "util/graph/tuple.h"

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

TEST(TupleTest, CanConstructOpaqueTuple) {
  auto opaque_tuple = OpaqueTuple::ConstructFromTypes<Unitary, Monotonic>();

  EXPECT_THAT(opaque_tuple.Types(),
              ::testing::ElementsAre(Type::kUnitary, Type::kMonotonic));
}

TEST(TupleTest, OpaqueTupleDiesOnAccessingEmpty) {
  auto opaque_tuple = OpaqueTuple::EmptyFromTypes<Unitary, Monotonic>();

  EXPECT_THAT(opaque_tuple.Types(),
              ::testing::ElementsAre(Type::kUnitary, Type::kMonotonic));

  EXPECT_DEATH({ opaque_tuple.Get<Unitary>(0) = 0.5f; }, "is nullptr");
}

TEST(TupleTest, CanStoreInOpaqueTuple) {
  auto opaque_tuple = OpaqueTuple::ConstructFromTypes<Unitary, Monotonic>();

  opaque_tuple.Get<Unitary>(0) = 0.123f;

  EXPECT_NEAR(opaque_tuple.Get<Unitary>(0), 0.123f, 1e-6f);
}

TEST(TupleTest, OpaqueTupleDiesOnIncorrectGet) {
  auto opaque_tuple = OpaqueTuple::ConstructFromTypes<Unitary, Monotonic>();

  EXPECT_DEATH({ opaque_tuple.Get<Monotonic>(0) = 10.0f; },
               "requesting incorrect type");
  EXPECT_DEATH({ opaque_tuple.Get<Monotonic>(5) = 10.0f; },
               "index out of bounds");
}

TEST(TupleTest, OpaqueTupleCanBeAssignedFromStdTuple) {
  auto opaque_tuple =
      OpaqueTuple::ConstructFromTypes<Unitary, Monotonic, Unitary>();

  auto tuple = std::tuple<Unitary, Monotonic, Unitary>(0.5f, 25.0f, 0.1f);
  opaque_tuple.AssignFrom(tuple);

  EXPECT_EQ(opaque_tuple.Get<Unitary>(0), 0.5f);
  EXPECT_EQ(opaque_tuple.Get<Monotonic>(1), 25.0f);
  EXPECT_EQ(opaque_tuple.Get<Unitary>(2), 0.1f);
}

//
// Tests covering aliasing.
//

TEST(TupleTest, OpaqueTupleCanAliasOtherTuple) {
  auto opaque_tuple = OpaqueTuple::ConstructFromTypes<int, float>();
  auto alias_opaque_tuple = OpaqueTuple::EmptyFromTypes<float, int>();

  alias_opaque_tuple.Alias(0, opaque_tuple, 1);
  alias_opaque_tuple.Alias(1, opaque_tuple, 0);

  opaque_tuple.AssignFrom(std::make_tuple(123, 0.123f));

  EXPECT_EQ(alias_opaque_tuple.Get<float>(0), 0.123f);
  EXPECT_EQ(alias_opaque_tuple.Get<int>(1), 123);

  // Aliasing is bidirectional.
  alias_opaque_tuple.Get<float>(0) = 0.456f;
  EXPECT_EQ(opaque_tuple.Get<float>(1), 0.456f);
}

TEST(TupleTest, OpaqueTupleDiesOnIncorrectAliasingTypes) {
  auto opaque_tuple = OpaqueTuple::ConstructFromTypes<int, float>();
  auto alias_opaque_tuple = OpaqueTuple::EmptyFromTypes<float, int>();

  EXPECT_DEATH({ alias_opaque_tuple.Alias(0, opaque_tuple, 0); },
               "incorrect type");
}

TEST(TupleTest, OpaqueTupleDiesOnOutOfBoundsIndex) {
  auto opaque_tuple = OpaqueTuple::ConstructFromTypes<int, float>();
  auto alias_opaque_tuple = OpaqueTuple::EmptyFromTypes<float, int>();

  EXPECT_DEATH({ alias_opaque_tuple.Alias(2, opaque_tuple, 0); },
               "out of bounds");
  EXPECT_DEATH({ alias_opaque_tuple.Alias(0, opaque_tuple, -1); },
               "out of bounds");
}

//
// Tests covering constructor/destructor behavior.
//

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

TEST(TupleTest, OpaqueTupleInvokesConstructorAndDestructor) {
  construction_state = ConstructionState::kUnconstructed;

  ASSERT_EQ(construction_state, ConstructionState::kUnconstructed);
  {
    auto opaque_tuple = OpaqueTuple::ConstructFromTypes<RaiiType>();
    EXPECT_EQ(construction_state, ConstructionState::kConstructed);
    EXPECT_EQ(opaque_tuple.Get<RaiiType>(0).member_with_default, 123);
  }
  EXPECT_EQ(construction_state, ConstructionState::kDestructed);
}

//
// Tests covering `OpaqueTupleFactory`.
//

TEST(TupleTest, OpaqueTupleFactoryConstructsTupleOfCorrectType) {
  auto factory = OpaqueTupleFactory::FromTypes<int, float>();

  OpaqueTuple tuple = factory.Construct();

  EXPECT_THAT(tuple.Types(),
              ::testing::ElementsAre(Type::kInteger, Type::kFloatGeneric));
}

TEST(TupleTest, OpaqueTupleFactoryConstructsDisjointTuples) {
  auto factory = OpaqueTupleFactory::FromTypes<int, float>();

  OpaqueTuple tuple_a = factory.Construct(), tuple_b = factory.Construct();

  tuple_a.Get<int>(0) = 5;
  tuple_b.Get<int>(0) = 3;

  EXPECT_EQ(tuple_a.Get<int>(0), 5);
  EXPECT_EQ(tuple_b.Get<int>(0), 3);
}

//
// Tests covering ToOpaqueFunction().
//

TEST(TupleTest, OpaqueTupleFunctionInvocation) {
  auto input_tuple = OpaqueTuple::ConstructFromTypes<float, float>();
  auto output_tuple = OpaqueTuple::ConstructFromTypes<float, int>();

  std::function<std::tuple<float, int>(std::tuple<float&, float&>)> convert_fn =
      [](std::tuple<float&, float&> in) -> std::tuple<float, int> {
    auto& [a, b] = in;
    return std::make_tuple(a + b, static_cast<int>(a * b));
  };
  auto opaque_fn = ToOpaqueFunction(convert_fn);

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
  auto opaque_fn = ToOpaqueFunction(convert_fn);

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

  auto opaque_a_fn = ToOpaqueFunction(a_fn);
  auto opaque_b_fn = ToOpaqueFunction(b_fn);
  auto opaque_c_fn = ToOpaqueFunction(c_fn);

  input_tuple.AssignFrom(std::make_tuple(10.0f, 3.5f));

  opaque_a_fn(input_tuple, mid_tuple_1);
  opaque_b_fn(input_tuple, mid_tuple_2);
  opaque_c_fn(mid_tuple_combined, output_tuple);

  EXPECT_EQ(output_tuple.Get<float>(0), 21.5f);
}

}  // namespace
}  // namespace opendrop
