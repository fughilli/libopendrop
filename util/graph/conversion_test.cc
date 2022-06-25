#include "util/graph/conversion.h"

#include <tuple>

#include "googlemock/include/gmock/gmock-matchers.h"
#include "googlemock/include/gmock/gmock-more-matchers.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "util/graph/tuple_factory.h"
#include "util/graph/types/monotonic.h"
#include "util/graph/types/types.h"
#include "util/graph/types/unitary.h"

namespace opendrop {
namespace {

//
// Tests covering ConversionToOpaqueFunction().
//

TEST(TupleTest, OpaqueTupleFunctionInvocation) {
  auto [input_storage, input_tuple] =
      OpaqueTupleFactory::StorageAndTupleFromTypes<float, float>();
  auto [output_storage, output_tuple] =
      OpaqueTupleFactory::StorageAndTupleFromTypes<float, int>();

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
  //           [A]                  [B]
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
  auto input_storage_1 = OpaqueStorable<float>::Allocate();
  auto input_storage_2 = OpaqueStorable<float>::Allocate();
  auto mid_storage_1 = OpaqueStorable<float>::Allocate();
  auto mid_storage_2 = OpaqueStorable<int>::Allocate();
  auto output_storage = OpaqueStorable<float>::Allocate();

  auto input_tuple =
      OpaqueTuple::FromTypesAndMemory({{Type::kFloatGeneric, input_storage_1},
                                       {Type::kFloatGeneric, input_storage_2}});
  auto mid_tuple_1 =
      OpaqueTuple::FromTypesAndMemory({{Type::kFloatGeneric, mid_storage_1}});
  auto mid_tuple_2 =
      OpaqueTuple::FromTypesAndMemory({{Type::kInteger, mid_storage_2}});
  auto mid_tuple_combined = OpaqueTuple::FromTypesAndMemory(
      {{Type::kInteger, mid_storage_2}, {Type::kFloatGeneric, mid_storage_1}});
  auto output_tuple =
      OpaqueTuple::FromTypesAndMemory({{Type::kFloatGeneric, output_storage}});

  EXPECT_TRUE(mid_tuple_combined.IsAliasOf(1, mid_tuple_1, 0));
  EXPECT_TRUE(mid_tuple_combined.IsAliasOf(0, mid_tuple_2, 0));

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

}  // namespace
}  // namespace opendrop
