#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;
varying vec2 texture_uv;
varying vec3 normal;

uniform sampler2D in_tex;
uniform float theta;
uniform int num_petals;

void main() {
  vec2 tex_uv =
      screen_to_tex(uv_warp_kaleidoscope(rotate(screen_uv, theta), num_petals));
  gl_FragColor = texture2D(in_tex, tex_uv);
}
