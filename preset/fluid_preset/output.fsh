#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;

uniform sampler2D render_target;
uniform ivec2 render_target_size;

void main() {
  vec2 texture_uv = screen_to_tex(screen_uv);

  vec4 cell = texture2D(render_target, texture_uv);
  vec2 velocity = cell.xy;
  float pressure = cell.z;

  float angle = atan(velocity.y, velocity.x);

  gl_FragColor = cell; //vec4(hsv_to_rgb(vec3(angle, pressure, length(velocity))), 1.0);
}
