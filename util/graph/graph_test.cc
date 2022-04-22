#include "util/graph/graph.h"

#include <cmath>
#include <tuple>

#include "googlemock/include/gmock/gmock-matchers.h"
#include "googlemock/include/gmock/gmock-more-matchers.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"
#include "util/graph/graph_builtins.h"
#include "util/graph/types/color.h"
#include "util/graph/types/monotonic.h"
#include "util/graph/types/types.h"
#include "util/graph/types/unitary.h"
#include "util/graphics/colors.h"
#include "util/graphics/gl_util.h"
#include "util/math/math.h"
#include "util/testing/graph_matchers.h"

namespace opendrop {
namespace {

TEST(GraphTest, SimpleConversion) {
  GraphBuilder graph_builder;
  graph_builder.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
      "sinusoid", [](std::tuple<Monotonic> in) -> std::tuple<Unitary> {
        return std::tuple<Unitary>(
            Unitary((1.0f + std::sin(std::get<0>(in))) / 2.0f));
      });

  Graph graph = graph_builder.Construct("sinusoid");

  graph.Evaluate(std::make_tuple<Monotonic>(0));
  EXPECT_NEAR(std::get<0>(graph.Result<Unitary>()), 0.5f, 1e-6f);
  graph.Evaluate(std::make_tuple<Monotonic>(kPi / 2));
  EXPECT_NEAR(std::get<0>(graph.Result<Unitary>()), 1.0f, 1e-6f);
}

TEST(GraphTest, SequenceConversion) {
  GraphBuilder graph_builder;
  graph_builder.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
      "sinusoid", [](std::tuple<Monotonic> in) -> std::tuple<Unitary> {
        return std::tuple<Unitary>(
            Unitary((1.0f + std::cos(std::get<0>(in))) / 2.0f));
      });
  graph_builder.DeclareConversion<std::tuple<Unitary>, std::tuple<Color>>(
      "color_wheel", [](std::tuple<Unitary> in) -> std::tuple<Color> {
        glm::vec4 color =
            glm::vec4(HsvToRgb(glm::vec3(std::get<0>(in), 1.0f, 1.0f)), 1.0f);
        return std::tuple<Color>(color);
      });

  auto value_or_graph = graph_builder.Bridge(ConstructTypes<Monotonic>(),
                                             ConstructTypes<Color>());
  EXPECT_TRUE(value_or_graph.ok()) << value_or_graph.status();
  Graph graph = std::move(value_or_graph).value_or(Graph{});
  graph.Evaluate(std::make_tuple<Monotonic>(0));

  EXPECT_THAT(std::get<0>(graph.Result<Color>()),
              graph_testing::ColorIsNear(
                  Color(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)), 0.001f));

  graph.Evaluate(std::make_tuple<Monotonic>(kPi / 2.0f));
  EXPECT_THAT(std::get<0>(graph.Result<Color>()),
              graph_testing::ColorIsNear(
                  Color(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)), 0.001f));
}

TEST(GraphTest, BuilderProducesEmptyGraphForUnsatisfiableConversion) {
  GraphBuilder graph_builder;
  graph_builder.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
      "unitary",
      [](std::tuple<Monotonic> in) -> std::tuple<Unitary> { return {}; });
  graph_builder.DeclareConversion<std::tuple<Unitary>, std::tuple<Color>>(
      "color", [](std::tuple<Unitary> in) -> std::tuple<Color> { return {}; });

  auto value_or_graph =
      graph_builder.Bridge(ConstructTypes<float>(), ConstructTypes<int>());
  EXPECT_FALSE(value_or_graph.ok());
}

TEST(GraphTest, UniqueGraphConversion) {
  // Compute a function graph:
  //
  // A := Monotonic(Unitary, float)     // Add
  // B := int(Unitary, float)           // Multiply, convert to `int`
  // C := Color(Monotonic, float, int)  // Send to channels
  //
  //        +-----------------------------+
  //        | input_tuple: Unitary, float |
  //        +-----------------------------+
  //            |                |      |
  //           [A]               |     [B]
  //            |                |      |
  //            |                |      V
  // +------------------------+  |  +------------------+
  // | mid_tuple_1: Monotonic |  |  | mid_tuple_2: int |
  // +------------------------+  |  +------------------+
  //            |                |      |
  //            |                |      |
  //            V                V      V
  // +--------------------------------------------+
  // | mid_tuple_combined : Monotonic, float, int |
  // +--------------------------------------------+
  //                      |
  //                     [C]
  //                      |
  //                      V
  //           +---------------------+
  //           | output_tuple: Color |
  //           +---------------------+
  //
  // `mid_tuple_1` and `mid_tuple_2` own their storage.
  //
  // `mid_tuple_combined` is an aliasing OpaqueTuple over both of them.

  std::function<std::tuple<float>(std::tuple<int&, float&>)> c_fn =
      [](std::tuple<int&, float&> in) -> std::tuple<float> {
    auto& [a, b] = in;
    return std::make_tuple(a - b);
  };

  GraphBuilder graph_builder;
  graph_builder
      .DeclareConversion<std::tuple<Unitary, float>, std::tuple<Monotonic>>(
          "add", [](std::tuple<Unitary, float> in) -> std::tuple<Monotonic> {
            auto& [u, f] = in;
            return std::make_tuple(u + f);
          });
  graph_builder.DeclareConversion<std::tuple<Unitary, float>, std::tuple<int>>(
      "multiply", [](std::tuple<Unitary, float> in) -> std::tuple<int> {
        auto& [u, f] = in;
        return std::make_tuple(static_cast<int>(u * f));
      });
  graph_builder
      .DeclareConversion<std::tuple<Monotonic, float, int>, std::tuple<Color>>(
          "to_channels",
          [](std::tuple<Monotonic, float, int> in) -> std::tuple<Color> {
            auto& [m, f, i] = in;

            float red = std::fmodf(m * f, 1.0f);
            float green = std::fmodf(m / i, 1.0f);
            return glm::vec4(red, green, 0.0f, 1.0f);
          });

  auto value_or_graph = graph_builder.Bridge(ConstructTypes<Unitary, float>(),
                                             ConstructTypes<Color>());
  ASSERT_TRUE(value_or_graph.ok()) << value_or_graph.status();

  Graph graph = std::move(value_or_graph).value_or(Graph{});
  graph.Evaluate(std::make_tuple<Unitary, float>(0.5f, 12.1f));

  EXPECT_THAT(std::get<0>(graph.Result<Color>()),
              graph_testing::ColorIsNear(
                  Color(glm::vec4(0.46f, 0.1f, 0.0f, 1.0f)), 0.001f));
}

}  // namespace
}  // namespace opendrop
