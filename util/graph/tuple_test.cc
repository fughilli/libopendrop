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
  std::pair<Type, std::shared_ptr<uint8_t>> types_and_memory[] = {
      {Type::kUnitary, Unitary::Allocate()},
      {Type::kMonotonic, Monotonic::Allocate()},
  };
  auto opaque_tuple = OpaqueTuple::FromTypesAndMemory(types_and_memory);

  EXPECT_THAT(opaque_tuple.Types(),
              ::testing::ElementsAre(Type::kUnitary, Type::kMonotonic));
  EXPECT_EQ(opaque_tuple.size(), 2);
}

TEST(TupleTest, OpaqueTupleDiesOnAccessingEmpty) {
  auto opaque_tuple = OpaqueTuple::EmptyFromTypes<Unitary, Monotonic>();

  EXPECT_THAT(opaque_tuple.Types(),
              ::testing::ElementsAre(Type::kUnitary, Type::kMonotonic));

  EXPECT_DEATH({ opaque_tuple.Ref<Unitary>(0) = 0.5f; }, "is null");
}

TEST(TupleTest, CanStoreInOpaqueTuple) {
  std::pair<Type, std::shared_ptr<uint8_t>> types_and_memory[] = {
      {Type::kUnitary, Unitary::Allocate()},
      {Type::kMonotonic, Monotonic::Allocate()},
  };
  auto opaque_tuple = OpaqueTuple::FromTypesAndMemory(types_and_memory);

  opaque_tuple.Ref<Unitary>(0) = 0.123f;

  EXPECT_NEAR(opaque_tuple.Get<Unitary>(0), 0.123f, 1e-6f);
}

TEST(TupleTest, OpaqueTupleDiesOnIncorrectGet) {
  std::pair<Type, std::shared_ptr<uint8_t>> types_and_memory[] = {
      {Type::kUnitary, Unitary::Allocate()},
      {Type::kMonotonic, Monotonic::Allocate()},
  };
  auto opaque_tuple = OpaqueTuple::FromTypesAndMemory(types_and_memory);

  EXPECT_DEATH({ opaque_tuple.Ref<Monotonic>(0) = 10.0f; },
               "requesting incorrect type");
  EXPECT_DEATH({ opaque_tuple.Ref<Monotonic>(5) = 10.0f; },
               "index out of bounds");
}

TEST(TupleTest, OpaqueTupleCanBeAssignedFromStdTuple) {
  std::pair<Type, std::shared_ptr<uint8_t>> types_and_memory[] = {
      {Type::kUnitary, Unitary::Allocate()},
      {Type::kMonotonic, Monotonic::Allocate()},
      {Type::kUnitary, Unitary::Allocate()},
  };
  auto opaque_tuple = OpaqueTuple::FromTypesAndMemory(types_and_memory);

  auto tuple = std::tuple<Unitary, Monotonic, Unitary>(0.5f, 25.0f, 0.1f);
  opaque_tuple.AssignFrom(tuple);

  EXPECT_EQ(opaque_tuple.Get<Unitary>(0), 0.5f);
  EXPECT_EQ(opaque_tuple.Get<Monotonic>(1), 25.0f);
  EXPECT_EQ(opaque_tuple.Get<Unitary>(2), 0.1f);

  EXPECT_EQ(opaque_tuple.size(), 3);
}

//
// Tests covering aliasing.
//

TEST(TupleTest, OpaqueTupleCanAliasOtherTuple) {
  std::pair<Type, std::shared_ptr<uint8_t>> types_and_memory[] = {
      {Type::kInteger, OpaqueStorable<int>::Allocate()},
      {Type::kFloatGeneric, OpaqueStorable<float>::Allocate()},
  };
  auto opaque_tuple = OpaqueTuple::FromTypesAndMemory(types_and_memory);
  auto alias_opaque_tuple = OpaqueTuple::EmptyFromTypes<float, int>();

  alias_opaque_tuple.Alias(0, opaque_tuple, 1);
  alias_opaque_tuple.Alias(1, opaque_tuple, 0);

  opaque_tuple.AssignFrom(std::make_tuple(123, 0.123f));

  EXPECT_EQ(alias_opaque_tuple.Get<float>(0), 0.123f);
  EXPECT_EQ(alias_opaque_tuple.Get<int>(1), 123);

  // Aliasing is bidirectional.
  alias_opaque_tuple.Ref<float>(0) = 0.456f;
  EXPECT_EQ(opaque_tuple.Get<float>(1), 0.456f);
}

TEST(TupleTest, OpaqueTupleIsAliasOfReturnsExpected) {
  std::pair<Type, std::shared_ptr<uint8_t>> types_and_memory[] = {
      {Type::kInteger, OpaqueStorable<int>::Allocate()},
      {Type::kFloatGeneric, OpaqueStorable<float>::Allocate()},
  };
  auto opaque_tuple = OpaqueTuple::FromTypesAndMemory(types_and_memory);
  auto alias_opaque_tuple = OpaqueTuple::EmptyFromTypes<float, int>();

  EXPECT_FALSE(alias_opaque_tuple.IsAliasOf(0, opaque_tuple, 1));
  EXPECT_FALSE(alias_opaque_tuple.IsAliasOf(1, opaque_tuple, 0));

  alias_opaque_tuple.Alias(0, opaque_tuple, 1);
  alias_opaque_tuple.Alias(1, opaque_tuple, 0);

  EXPECT_TRUE(alias_opaque_tuple.IsAliasOf(0, opaque_tuple, 1));
  EXPECT_TRUE(alias_opaque_tuple.IsAliasOf(1, opaque_tuple, 0));
}

TEST(TupleTest, OpaqueTupleDiesOnIncorrectAliasingTypes) {
  std::pair<Type, std::shared_ptr<uint8_t>> types_and_memory[] = {
      {Type::kInteger, OpaqueStorable<int>::Allocate()},
      {Type::kFloatGeneric, OpaqueStorable<float>::Allocate()},
  };
  auto opaque_tuple = OpaqueTuple::FromTypesAndMemory(types_and_memory);
  auto alias_opaque_tuple = OpaqueTuple::EmptyFromTypes<float, int>();

  EXPECT_DEATH({ alias_opaque_tuple.Alias(0, opaque_tuple, 0); },
               "incorrect type");
}

TEST(TupleTest, OpaqueTupleDiesOnOutOfBoundsIndex) {
  std::pair<Type, std::shared_ptr<uint8_t>> types_and_memory[] = {
      {Type::kInteger, OpaqueStorable<int>::Allocate()},
      {Type::kFloatGeneric, OpaqueStorable<float>::Allocate()},
  };
  auto opaque_tuple = OpaqueTuple::FromTypesAndMemory(types_and_memory);
  auto alias_opaque_tuple = OpaqueTuple::EmptyFromTypes<float, int>();

  EXPECT_DEATH({ alias_opaque_tuple.Alias(2, opaque_tuple, 0); },
               "out of bounds");
  EXPECT_DEATH({ alias_opaque_tuple.Alias(0, opaque_tuple, -1); },
               "out of bounds");
}

}  // namespace
}  // namespace opendrop
