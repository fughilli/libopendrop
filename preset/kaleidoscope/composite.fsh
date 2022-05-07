#version 120

#include "preset/common/math.shh"

uniform sampler2D render_target;
uniform ivec2 render_target_size;

varying vec2 screen_uv;

void main() {
  vec2 tex_uv = screen_to_tex(screen_uv);
  gl_FragColor = texture2D(render_target, tex_uv);
}
