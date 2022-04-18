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
  ComputeGraph graph;
  graph.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
      "sinusoid", [](std::tuple<Monotonic> in) -> std::tuple<Unitary> {
        return std::tuple<Unitary>(
            Unitary((1.0f + std::sin(std::get<0>(in))) / 2.0f));
      });

  Graph g = graph.Construct("sinusoid");

  g.Evaluate(std::make_tuple<Monotonic>(0));
  EXPECT_NEAR(std::get<0>(g.Result<Unitary>()), 0.5f, 1e-6f);
  g.Evaluate(std::make_tuple<Monotonic>(kPi / 2));
  EXPECT_NEAR(std::get<0>(g.Result<Unitary>()), 1.0f, 1e-6f);
}

TEST(GraphTest, SequenceConversion) {
  ComputeGraph graph;
  graph.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
      "sinusoid", [](std::tuple<Monotonic> in) -> std::tuple<Unitary> {
        return std::tuple<Unitary>(
            Unitary((1.0f + std::cos(std::get<0>(in))) / 2.0f));
      });
  graph.DeclareConversion<std::tuple<Unitary>, std::tuple<Color>>(
      "color_wheel", [](std::tuple<Unitary> in) -> std::tuple<Color> {
        glm::vec4 color =
            glm::vec4(HsvToRgb(glm::vec3(std::get<0>(in), 1.0f, 1.0f)), 1.0f);
        return std::tuple<Color>(color);
      });

  Graph g = graph.Bridge(ConstructTypes<Monotonic>(), ConstructTypes<Color>());
  g.Evaluate(std::make_tuple<Monotonic>(0));

  EXPECT_THAT(std::get<0>(g.Result<Color>()),
              graph_testing::ColorIsNear(
                  Color(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)), 0.001f));

  g.Evaluate(std::make_tuple<Monotonic>(kPi / 2.0f));
  EXPECT_THAT(std::get<0>(g.Result<Color>()),
              graph_testing::ColorIsNear(
                  Color(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)), 0.001f));
}

}  // namespace
}  // namespace opendrop
