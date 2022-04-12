#include "util/graph/graph.h"

#include <cmath>
#include <tuple>

#include "googlemock/include/gmock/gmock-matchers.h"
#include "googlemock/include/gmock/gmock-more-matchers.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

namespace opendrop {
namespace {

TEST(GraphTest, ToTypeTest) {
  EXPECT_EQ(ToType<float>(), Type::kFloatGeneric);
  EXPECT_EQ(ToType<Unitary>(), Type::kUnitary);
  EXPECT_EQ(ToType<Monotonic>(), Type::kMonotonic);
}

TEST(GraphTest, ConstructTypesTest) {
  EXPECT_THAT(ConstructTypesFromTuple(std::tuple<>()), ::testing::IsEmpty());
  EXPECT_THAT(ConstructTypesFromTuple(std::tuple<Monotonic, Unitary>()),
              ::testing::ElementsAre(Type::kMonotonic, Type::kUnitary));
  EXPECT_THAT(
      ConstructTypesFromTuple(std::tuple<Unitary, Monotonic, Unitary>()),
      ::testing::ElementsAre(Type::kUnitary, Type::kMonotonic, Type::kUnitary));
}

TEST(GraphTest, CanConstructConversion) {
  std::function<std::tuple<float>(std::tuple<float>)> convert_fn =
      [](std::tuple<float> in) -> std::tuple<float> {
    return std::make_tuple(std::get<0>(in));
  };
  auto conversion = std::make_shared<Conversion>("foobar", convert_fn);
}

TEST(GraphTest, CanInvokeConversion) {
  std::function<std::tuple<float>(std::tuple<float>)> convert_fn =
      [](std::tuple<float> in) -> std::tuple<float> {
    return std::make_tuple(std::get<0>(in) * 2);
  };
  auto conversion = std::make_shared<Conversion>("times2", convert_fn);

  EXPECT_NEAR(std::get<0>(conversion->Invoke(std::tuple<float>(123.123f))
                              .Result<std::tuple<float>>()),
              246.246f, 1e-6f);
}

// TEST(GraphTest, SimpleConversion) {
//   ComputeGraph graph;
//   graph.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
//      "sinusoid", [](std::tuple<Monotonic> in) -> std::tuple<Unitary> {
//         return std::tuple<Unitary>(
//             Unitary((1.0f + std::sin(std::get<0>(in))) / 2.0f));
//        return std::tuple<Unitary>(Unitary(0));
//      });
//
//  // graph.Construct("sinusoid");
//
//  // EXPECT_NEAR(
//  //     graph.Evaluate<std::tuple<Unitary>>(std::make_tuple<Monotonic>(0)),
//  0);
//  // EXPECT_NEAR(
//  //     graph.Evaluate<std::tuple<Unitary>>(std::make_tuple<Monotonic>(kPi /
//  //     2)), 1);
//}

}  // namespace
}  // namespace opendrop
