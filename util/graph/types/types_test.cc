#include "util/graph/types/types.h"

#include <tuple>

#include "googlemock/include/gmock/gmock-matchers.h"
#include "googlemock/include/gmock/gmock-more-matchers.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "util/graph/types/monotonic.h"
#include "util/graph/types/unitary.h"

namespace opendrop {
namespace {

TEST(TypesTest, ToTypeTest) {
  EXPECT_EQ(ToType<float>(), Type::kFloatGeneric);
  EXPECT_EQ(ToType<Unitary>(), Type::kUnitary);
  EXPECT_EQ(ToType<Monotonic>(), Type::kMonotonic);
}

TEST(TypesTest, ConstructTypesTest) {
  EXPECT_THAT(ConstructTypesFromTuple(std::tuple<>()), ::testing::IsEmpty());
  EXPECT_THAT(ConstructTypesFromTuple(std::tuple<Monotonic, Unitary>()),
              ::testing::ElementsAre(Type::kMonotonic, Type::kUnitary));
  EXPECT_THAT(
      ConstructTypesFromTuple(std::tuple<Unitary, Monotonic, Unitary>()),
      ::testing::ElementsAre(Type::kUnitary, Type::kMonotonic, Type::kUnitary));
}

}  // namespace
}  // namespace opendrop
