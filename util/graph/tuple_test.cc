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
  auto opaque_tuple = OpaqueTuple::FromTypes<Unitary, Monotonic>();

  EXPECT_THAT(opaque_tuple.types,
              ::testing::ElementsAre(Type::kUnitary, Type::kMonotonic));
  EXPECT_EQ(opaque_tuple.cells.size(), 2);
  EXPECT_EQ(opaque_tuple.cells[0].type, Type::kUnitary);
  EXPECT_EQ(opaque_tuple.cells[1].type, Type::kMonotonic);
}

TEST(TupleTest, CanStoreInOpaqueTuple) {
  auto opaque_tuple = OpaqueTuple::FromTypes<Unitary, Monotonic>();

  opaque_tuple.Get<Unitary>(0) = 0.123f;

  EXPECT_NEAR(opaque_tuple.Get<Unitary>(0), 0.123f, 1e-6f);
}

TEST(TupleTest, OpaqueTupleDiesOnIncorrectGet) {
  auto opaque_tuple = OpaqueTuple::FromTypes<Unitary, Monotonic>();

  EXPECT_DEATH({ opaque_tuple.Get<Monotonic>(0) = 10.0f; },
               "requesting incorrect type");
  EXPECT_DEATH({ opaque_tuple.Get<Monotonic>(5) = 10.0f; },
               "index out of bounds");
}

TEST(TupleTest, OpaqueTupleCanBeAssignedFromStdTuple) {
  auto opaque_tuple = OpaqueTuple::FromTypes<Unitary, Monotonic, Unitary>();

  auto tuple = std::tuple<Unitary, Monotonic, Unitary>(0.5f, 25.0f, 0.1f);
  opaque_tuple.AssignFrom(tuple);

  EXPECT_EQ(opaque_tuple.Get<Unitary>(0), 0.5f);
  EXPECT_EQ(opaque_tuple.Get<Monotonic>(1), 25.0f);
  EXPECT_EQ(opaque_tuple.Get<Unitary>(2), 0.1f);
}

TEST(TupleTest, OpaqueTupleFunctionInvocation) {
  auto input_tuple = OpaqueTuple::FromTypes<float, float>();
  auto output_tuple = OpaqueTuple::FromTypes<float, int>();

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

}  // namespace
}  // namespace opendrop
