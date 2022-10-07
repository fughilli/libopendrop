#include "util/graphics/gl_texture_manager.h"

#include <stddef.h>

#include <string>

#include "third_party/gl_helper.h"
#include "util/logging/logging.h"

namespace gl {

namespace {
std::string MakeAllocationString(const std::vector<bool>& allocated) {
  std::string s(allocated.size(), '_');
  for (size_t i = 0; i < allocated.size(); ++i) s[i] = allocated[i] ? 'X' : '_';

  return s;
}
}  // namespace

GlTextureManager::GlTextureManager() : total_texture_units_(0) {
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &total_texture_units_);

  allocated_texture_units_.resize(total_texture_units_, false);

  for (int i = 0; i < total_texture_units_; ++i) {
    free_texture_units_.push(i);
  }
}

void GlTextureManager::PrintState() const {
  LOG_N_SEC(1.0, INFO) << "Allocated texture units: "
                       << MakeAllocationString(allocated_texture_units_);
}

absl::StatusOr<int> GlTextureManager::Allocate() {
  if (free_texture_units_.empty()) {
    return absl::FailedPreconditionError("No free texture units available.");
  }

  int texture_unit = free_texture_units_.front();
  free_texture_units_.pop();

  allocated_texture_units_[texture_unit] = true;

  LOG(INFO) << "Allocated texture unit " << texture_unit;

  PrintState();

  return texture_unit;
}

void GlTextureManager::Deallocate(int texture_unit) {
  CHECK(0 <= texture_unit && texture_unit < total_texture_units_)
      << "Invalid texture unit index.";
  CHECK(allocated_texture_units_[texture_unit])
      << "Texture unit " << texture_unit << " is not allocated.";

  LOG(DEBUG) << "Deallocated texture unit: " << texture_unit;
  allocated_texture_units_[texture_unit] = false;
  free_texture_units_.push(texture_unit);
}

}  // namespace gl
