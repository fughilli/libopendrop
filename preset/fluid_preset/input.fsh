#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;

uniform sampler2D input_frame;
uniform ivec2 size;

void main() {
  vec2 texture_uv = screen_to_tex(screen_uv);

  gl_FragColor = gl_Color; //mix(gl_Color, texture2D(input_frame, texture_uv), gl_Color.a);
}
