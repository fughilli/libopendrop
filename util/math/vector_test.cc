#include "util/math/vector.h"

#include <iostream>

#include "googletest/include/gtest/gtest.h"
#include "util/glm_helper.h"
#include "util/logging.h"
#include "util/math.h"
#include "util/testing/glm_matchers.h"

namespace opendrop {

namespace {

using ::opendrop::glm_testing::IsNear;

TEST(VectorTest, UnitVectorAtAngleReturnsExpectedValue) {
  EXPECT_THAT(UnitVectorAtAngle(0), IsNear(glm::vec2(1.0f, 0.0f), kEpsilon));
  EXPECT_THAT(UnitVectorAtAngle(kPi / 2),
              IsNear(glm::vec2(0.0f, 1.0f), kEpsilon));
  EXPECT_THAT(UnitVectorAtAngle(kPi), IsNear(glm::vec2(-1.0f, 0.0f), kEpsilon));
  EXPECT_THAT(UnitVectorAtAngle(3 * kPi / 2),
              IsNear(glm::vec2(0.0f, -1.0f), kEpsilon));
}

TEST(VectorTest, GlmMatricesAreOfExpectedOrientation) {
  const auto translation_matrix = glm::mat4(1, 0, 0, 0,  // Col 1
                                            0, 1, 0, 0,  // Col 2
                                            0, 0, 1, 0,  // Col 4
                                            3, 4, 5, 1   // Col 5
  );
  const auto vector = glm::vec3(7, 8, 9);
  EXPECT_THAT((translation_matrix * glm::vec4(vector, 1.0f)).xyz(),
              IsNear(glm::vec3(10.0f, 12.0f, 14.0f), 0.1f));
}

TEST(VectorTest, RingTransformReturnsExpectedValue) {
  EXPECT_THAT(RingTransform(glm::vec3(0, 0, -1), 1, 0),
              IsNear(glm::mat4(1, 0, 0, 0,  // Col 1
                               0, 1, 0, 0,  // Col 2
                               0, 0, 1, 0,  // Col 3
                               1, 0, 0, 1   // Col 4
                               ),
                     0.1f));
  EXPECT_THAT(RingTransform(glm::vec3(0, 0, -1), 1, 0.25),
              IsNear(glm::mat4(0, -1, 0, 0,  // Col 1
                               1, 0, 0, 0,   // Col 2
                               0, 0, 1, 0,   // Col 3
                               0, -1, 0, 1   // Col 4
                               ),
                     0.1f));
  EXPECT_THAT(RingTransform(glm::vec3(0, 0, -1), 1, 0.5),
              IsNear(glm::mat4(-1, 0, 0, 0,  // Col 1
                               0, -1, 0, 0,  // Col 2
                               0, 0, 1, 0,   // Col 3
                               -1, 0, 0, 1   // Col 4
                               ),
                     0.1f));
  EXPECT_THAT(RingTransform(glm::vec3(0, 0, -1), 1, 0.75),
              IsNear(glm::mat4(0, 1, 0, 0,   // Col 1
                               -1, 0, 0, 0,  // Col 2
                               0, 0, 1, 0,   // Col 3
                               0, 1, 0, 1    // Col 4
                               ),
                     0.1f));
  EXPECT_THAT(RingTransform(glm::vec3(0, 0, -1), 1, 1),
              IsNear(glm::mat4(1, 0, 0, 0,  // Col 1
                               0, 1, 0, 0,  // Col 2
                               0, 0, 1, 0,  // Col 3
                               1, 0, 0, 1   // Col 4
                               ),
                     0.1f));
}

TEST(VectorTest, ExtractTransformComponentsReturnsExpectedValues) {
  auto [scale, rotation, translation] =
      ExtractTransformComponents(glm::mat4(0, -1, 0, 0,  // Col 1
                                           2, 0, 0, 0,   // Col 2
                                           0, 0, 3, 0,   // Col 3
                                           3, 4, 5, 1    // Col 4

                                           ));
  EXPECT_THAT(scale, IsNear(glm::mat4(1, 0, 0, 0,  // Col 1
                                      0, 2, 0, 0,  // Col 2
                                      0, 0, 3, 0,  // Col 3
                                      0, 0, 0, 1   // Col 4
                                      ),

                            kEpsilon));
  EXPECT_THAT(rotation, IsNear(glm::mat4(0, -1, 0, 0,  // Col 1
                                         1, 0, 0, 0,   // Col 2
                                         0, 0, 1, 0,   // Col 3
                                         0, 0, 0, 1    // Col 4
                                         ),
                               kEpsilon));
  EXPECT_THAT(translation, IsNear(glm::mat4(1, 0, 0, 0,  // Col 1
                                            0, 1, 0, 0,  // Col 2
                                            0, 0, 1, 0,  // Col 3
                                            3, 4, 5, 1   // Col 4
                                            ),
                                  kEpsilon));
}

}  // namespace
}  // namespace opendrop
