#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;
varying vec2 texture_uv;
varying vec3 normal;

uniform sampler2D in_tex_a;
uniform sampler2D in_tex_b;

void main() {
  vec2 tex_a_uv = screen_to_tex(screen_uv);
  vec2 tex_b_uv = screen_to_tex(screen_uv * 0.99);

  gl_FragColor =
      mix(texture2D(in_tex_a, tex_a_uv), texture2D(in_tex_b, tex_b_uv), 0.5);
}
