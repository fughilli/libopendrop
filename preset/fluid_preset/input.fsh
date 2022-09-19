#version 120

#include "preset/common/math.shh"

varying vec2 texture_uv;
uniform sampler2D last_frame;
uniform vec2 splat_center;
uniform vec2 splat_velocity;

const float kDistance = 0.05;

void main() {
  vec4 last_fragment_color = texture2D(last_frame, texture_uv);

  float distance = length(texture_uv - screen_to_tex(splat_center));
  if (distance > kDistance) {
    gl_FragColor = last_fragment_color;
  } else {
    gl_FragColor =
        vec4(mix(splat_velocity, last_fragment_color.xy, distance / kDistance),
             0.0, 1.0);
  }
}
