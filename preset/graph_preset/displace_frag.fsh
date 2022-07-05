#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;
varying vec2 texture_uv;
varying vec3 normal;

uniform sampler2D in_tex;
uniform vec2 displacement;

void main() {
  vec2 tex_uv = screen_to_tex(screen_uv + displacement);
  gl_FragColor = texture2D(in_tex, tex_uv);
}
