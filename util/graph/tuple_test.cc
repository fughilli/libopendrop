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
}

}  // namespace
}  // namespace opendrop
