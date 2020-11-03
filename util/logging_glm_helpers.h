#ifndef UTIL_LOGGING_GLM_HELPERS_H_
#define UTIL_LOGGING_GLM_HELPERS_H_

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const glm::vec2& vec);
std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);
std::ostream& operator<<(std::ostream& os, const glm::vec4& vec);

#endif  // UTIL_LOGGING_GLM_HELPERS_H_
