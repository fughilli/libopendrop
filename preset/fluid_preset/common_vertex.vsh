#version 120

#include "preset/common/math.shh"

varying vec2 texture_uv;
varying vec2 l_uv;
varying vec2 r_uv;
varying vec2 u_uv;
varying vec2 d_uv;

uniform ivec2 size;

void main() {
  texture_uv = screen_to_tex(ftransform().xy);
  vec2 texel_size = vec2(1.0 / size.x, 1.0 / size.y);
  l_uv = texture_uv - vec2(texel_size.x, 0);
  r_uv = texture_uv + vec2(texel_size.x, 0);
  u_uv = texture_uv + vec2(0, texel_size.y);
  d_uv = texture_uv - vec2(0, texel_size.y);
  gl_Position = ftransform();
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
}
