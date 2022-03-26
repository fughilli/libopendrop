#ifndef UTIL_LOGGING_LOGGING_GLM_HELPERS_H_
#define UTIL_LOGGING_LOGGING_GLM_HELPERS_H_

#include <iostream>

#include "third_party/glm_helper.h"

std::ostream& operator<<(std::ostream& os, const glm::vec2& vec);
std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);
std::ostream& operator<<(std::ostream& os, const glm::vec4& vec);
std::ostream& operator<<(std::ostream& os, const glm::mat4& vec);

#endif  // UTIL_LOGGING_LOGGING_GLM_HELPERS_H_
