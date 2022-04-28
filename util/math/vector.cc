#include <cmath>
#include <tuple>

#include "third_party/glm_helper.h"
#include "util/logging/logging.h"
#include "util/math/constants.h"
#include "util/math/math.h"
#include "util/math/perspective.h"

namespace opendrop {

glm::vec2 UnitVectorAtAngle(float angle) {
  return glm::vec2(std::cos(angle), std::sin(angle));
}

glm::vec2 Rotate2d(glm::vec2 vector, float angle) {
  float cos_angle = std::cos(angle);
  float sin_angle = std::sin(angle);

  return glm::vec2(vector.x * cos_angle - vector.y * sin_angle,
                   vector.x * sin_angle + vector.y * cos_angle);
}

glm::mat4 RingTransform(glm::vec3 normal, float radius,
                        float position_along_ring) {
  glm::mat4 translation = Constants::kI4x4;
  glm::vec3 start_position = glm::cross(normal, Directions::kUp);
  translation[3] = glm::vec4(start_position * radius, 1.0f);
  return glm::rotate(Constants::kI4x4, position_along_ring * kPi * 2.0f,
                     normal) *
         translation;
}

glm::mat4 TranslationTransform(glm::vec3 translation) {
  glm::mat4 ret = Constants::kI4x4;
  ret[3] = glm::vec4(translation, 1.0f);
  return ret;
}

glm::mat4 LineTransform(glm::vec3 axis, float line_length,
                        float position_along_line) {
  glm::mat4 translation = Constants::kI4x4;
  translation[3] = glm::vec4(
      axis *
          (line_length / 2.0f * -std::clamp(position_along_line, 0.0f, 1.0f)),
      1.0f);
  return translation;
}

glm::mat4 RotateAround(glm::vec3 axis, float angle) {
  return glm::rotate(Constants::kI4x4, angle, axis);
}

std::tuple<glm::mat4, glm::mat4, glm::mat4> ExtractTransformComponents(
    glm::mat4 t) {
  glm::mat4 translation(1);
  // Copy the translation from t (column 3)
  translation[3] = t[3];

  glm::mat3 upper_3x3 = glm::mat3(t);

  // Compute the scale from the sqrt(Trace(t))
  const float scale_x = glm::length(upper_3x3[0]);
  const float scale_y = glm::length(upper_3x3[1]);
  const float scale_z = glm::length(upper_3x3[2]);

  const glm::mat4 scale = glm::mat4(scale_x, 0, 0, 0,  // Row 1
                                    0, scale_y, 0, 0,  // Row 2
                                    0, 0, scale_z, 0,  // Row 3
                                    0, 0, 0, 1);

  // Copy the rotation from t (upper 3x3)
  const glm::mat4 rotation(glm::vec4(upper_3x3[0] / scale_x, 0),
                           glm::vec4(upper_3x3[1] / scale_y, 0),
                           glm::vec4(upper_3x3[2] / scale_z, 0),
                           glm::vec4(0, 0, 0, 1));

  return std::make_tuple(scale, rotation, translation);
}

glm::mat4 ScaleTransform(float scale) {
  return glm::mat4(scale, 0, 0, 0,  // Row 1
                   0, scale, 0, 0,  // Row 2
                   0, 0, scale, 0,  // Row 3
                   0, 0, 0, 1       // Row 4
  );
}

glm::mat4 ScaleTransform(float scale_x, float scale_y, float scale_z) {
  return glm::mat4(scale_x, 0, 0, 0,  // Row 1
                   0, scale_y, 0, 0,  // Row 2
                   0, 0, scale_z, 0,  // Row 3
                   0, 0, 0, 1         // Row 4
  );
}

glm::mat4 AssembleTransform(float scale, glm::mat4 rotation,
                            glm::mat4 translation) {
  return translation * rotation * ScaleTransform(scale);
}
glm::mat4 AssembleTransform(glm::mat4 scale, glm::mat4 rotation,
                            glm::mat4 translation) {
  return translation * rotation * scale;
}

glm::mat4 TransformMix(glm::mat4 t1, glm::mat4 t2, float coeff) {
  auto [scale1, rotation1, translation1] = ExtractTransformComponents(t1);
  auto [scale2, rotation2, translation2] = ExtractTransformComponents(t2);

  glm::mat4 new_scale = Lerp(scale1, scale2, coeff);
  glm::mat4 new_translation = Lerp(translation1, translation2, coeff);
  glm::mat4 new_rotation = glm::toMat4(
      glm::mix(glm::quat_cast(rotation1), glm::quat_cast(rotation2), coeff));

  return AssembleTransform(new_scale, new_rotation, new_translation);
}

float EvenDistribution(int index, int count) {
  return static_cast<float>(index) / count;
}

float ClusterDistribution(int index, int count, int n_clusters,
                          float cluster_scale, float* out_cluster_coeff) {
  const int cluster_index =
      std::floor(EvenDistribution(index, count) * n_clusters);
  const float cluster_start = EvenDistribution(cluster_index, n_clusters);
  const float cluster_end = EvenDistribution(cluster_index + 1, n_clusters);

  const int cluster_start_index = std::floor(cluster_start * count);
  const int cluster_end_index = std::floor(cluster_end * count);

  const float aligned_cluster_start =
      EvenDistribution(cluster_start_index, count);
  const float aligned_cluster_end = EvenDistribution(cluster_end_index, count);

  const float cluster_center =
      (aligned_cluster_start + aligned_cluster_end) / 2.0f;
  const float cluster_width =
      (aligned_cluster_end - aligned_cluster_start) * cluster_scale;

  if (out_cluster_coeff != nullptr) {
    *out_cluster_coeff = MapValue<float>(index, cluster_start_index,
                                         cluster_end_index, 0.0f, 1.0f);
  }
  return MapValue<float>(index, cluster_start_index, cluster_end_index,
                         cluster_center - cluster_width / 2.0f,
                         cluster_center + cluster_width / 2.0f);
}

// def ClusterDistribution(index, count, n_clusters, cluster_scale):
//     cluster_index = round(EvenDistribution(index, count) * n_clusters)
//     cluster_start = EvenDistribution(cluster_index, n_clusters)
//     cluster_end = EvenDistribution(cluster_index + 1, n_clusters)
//
//     cluster_start_index = round(cluster_start * (count))
//     cluster_end_index = round(cluster_end * (count))
//
//     cluster_start = EvenDistribution(cluster_start_index, count)
//     cluster_end = EvenDistribution(cluster_end_index, count)
//
//     cluster_center = (cluster_start + cluster_end) / 2
//     cluster_width = (cluster_end - cluster_start) * cluster_scale
//
//     return (Map(index, cluster_start_index, cluster_end_index,
//                 cluster_center - cluster_width / 2,
//                 cluster_center + cluster_width / 2),
//             Map(index, cluster_start_index, cluster_end_index, 0, 1))

}  // namespace opendrop
