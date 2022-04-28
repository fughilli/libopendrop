#ifndef UTIL_MATH_VECTOR_H_
#define UTIL_MATH_VECTOR_H_

#include <tuple>

#include "third_party/glm_helper.h"

namespace opendrop {

glm::vec2 UnitVectorAtAngle(float angle);

// Rotates a vector counterclockwise by an angle in radians.
glm::vec2 Rotate2d(glm::vec2 vector, float angle);

// Generates a transform that takes an object to a position along a ring of
// radius `radius`. The position along the ring is given in turns; when
// `position_along_ring` is 0, the object is positioned at <radius, 0>. The
// object moves counterclockwise with increasing `position_along_ring`.
glm::mat4 RingTransform(glm::vec3 normal, float radius,
                        float position_along_ring);

// Generates a transform that takes an object to a position
glm::mat4 LineTransform(glm::vec3 start_position, glm::vec3 end_position,
                        float position_along_line);
glm::mat4 LineTransform(glm::vec3 axis, float line_length,
                        float position_along_line);

// Extracts the scale, rotation, and translation components of a transform.
std::tuple<glm::mat4, glm::mat4, glm::mat4> ExtractTransformComponents(
    glm::mat4 t);

// Computes a scaling matrix with no other components.
glm::mat4 ScaleTransform(float scale);

// Assembles a transformation matrix from components.
glm::mat4 AssembleTransform(float scale, glm::mat4 rotation,
                            glm::mat4 translation);
glm::mat4 AssembleTransform(glm::mat4 scale, glm::mat4 rotation,
                            glm::mat4 translation);

// Mixes two transforms by linearly interpolating between their components.
//
// The scale is linearly interpolated.
// The rotation is SLERP'd (spherically linearly interpolated).
// The translation is linearly interpolated.
glm::mat4 TransformMix(glm::mat4 t1, glm::mat4 t2, float coeff);

// Computes an evenly distributed position from an index and a count.
float EvenDistribution(int index, int count);

// Computes a position coefficient for a number of clusters, `n_clusters`, whose
// sizes are scaled by `cluster_scale`. When `cluster_scale` is 1.0f, this is
// equivalent to EvenDistribution().
float ClusterDistribution(int index, int count, int n_clusters,
                          float cluster_scale, float* out_cluster_coeff);

glm::mat4 RotateAround(glm::vec3 axis, float angle);

}  // namespace opendrop

#endif  // UTIL_MATH_VECTOR_H_
