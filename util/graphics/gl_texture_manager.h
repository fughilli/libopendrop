#ifndef UTIL_GRAPHICS_GL_TEXTURE_MANAGER_H_
#define UTIL_GRAPHICS_GL_TEXTURE_MANAGER_H_

#include <mutex>
#include <queue>
#include <vector>

#include "absl/status/statusor.h"

namespace gl {

class GlTextureManager {
 public:
  GlTextureManager();

  absl::StatusOr<int> Allocate();
  void Deallocate(int texture_unit);

  void PrintState() const;

 private:
  int total_texture_units_;
  std::mutex texture_unit_mu_;
  std::queue<int> free_texture_units_;
  std::vector<bool> allocated_texture_units_;
};

}  // namespace gl

#endif  // UTIL_GRAPHICS_GL_TEXTURE_MANAGER_H_
