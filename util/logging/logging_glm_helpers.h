#ifndef UTIL_LOGGING_LOGGING_GLM_HELPERS_H_
#define UTIL_LOGGING_LOGGING_GLM_HELPERS_H_

#include <iostream>

#include "third_party/glm_helper.h"

namespace glm {

std::ostream& operator<<(std::ostream& os, const vec2& vec);
std::ostream& operator<<(std::ostream& os, const vec3& vec);
std::ostream& operator<<(std::ostream& os, const vec4& vec);
std::ostream& operator<<(std::ostream& os, const mat4& vec);

}  // namespace glm

#endif  // UTIL_LOGGING_LOGGING_GLM_HELPERS_H_
