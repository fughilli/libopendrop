#version 120

#include "preset/common/math.shh"

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform float power;
uniform float energy;

varying vec2 screen_uv;

void main() {
  vec2 screen_uv_p = rotate(screen_uv, energy * 5);

  vec2 tex_uv = screen_to_tex(screen_uv_p);
  gl_FragColor = texture2D(render_target, tex_uv);
}
