#include "util/logging_glm_helpers.h"

#include <iostream>

#include "absl/strings/str_cat.h"
#include "googletest/include/gtest/gtest.h"
#include "util/glm_helper.h"
#include "util/testing/re2_matchers.h"

namespace opendrop {
namespace {

using re2_testing::MatchesRegex;

TEST(LoggingGlmHelperTest, LogVec2ReturnsExpected) {
  std::stringstream ss;
  ss << glm::vec2(1.0f, 2.0f);

  EXPECT_THAT(ss.str(), MatchesRegex("<1(\\.0\\+)?, 2(\\.0\\+)?>"));
}

TEST(LoggingGlmHelperTest, LogVec3ReturnsExpected) {
  std::stringstream ss;
  ss << glm::vec3(1.0f, 2.0f, 3.0f);

  EXPECT_THAT(ss.str(),
              MatchesRegex("<1(\\.0\\+)?, 2(\\.0\\+)?, 3(\\.0\\+)?>"));
}

TEST(LoggingGlmHelperTest, LogVec4ReturnsExpected) {
  std::stringstream ss;
  ss << glm::vec4(1.0f, 2.0f, 3.0f, 4.0f);

  EXPECT_THAT(
      ss.str(),
      MatchesRegex("<1(\\.0\\+)?, 2(\\.0\\+)?, 3(\\.0\\+)?, 4(\\.0\\+)?>"));
}

TEST(LoggingGlmHelperTest, LogMat4ReturnsExpected) {
  std::stringstream ss;
  ss << glm::mat4(1, 5, 9, 13,   // Col 1
                  2, 6, 10, 14,  // Col 2
                  3, 7, 11, 15,  // Col 3
                  4, 8, 12, 16   // Col 4
  );

  constexpr char kNotNum[] = "[0. ]+";

  EXPECT_THAT(ss.str(), MatchesRegex(absl::StrCat(
                            "\\|", kNotNum, "1", kNotNum, "2", kNotNum, "3",
                            kNotNum, "4", kNotNum, "\\|\n",  // Row 1
                            "\\|", kNotNum, "5", kNotNum, "6", kNotNum, "7",
                            kNotNum, "8", kNotNum, "\\|\n",  // Row 2
                            "\\|", kNotNum, "9", kNotNum, "10", kNotNum, "11",
                            kNotNum, "12", kNotNum, "\\|\n",  // Row 3
                            "\\|", kNotNum, "13", kNotNum, "14", kNotNum, "15",
                            kNotNum, "16", kNotNum, "\\|\n"  // Row 4
                            )));
}

}  // namespace
}  // namespace opendrop
