#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;

uniform sampler2D input_frame;
uniform ivec2 input_frame_size;

void main() {
  vec2 texture_uv = screen_to_tex(screen_uv);

  gl_FragColor = texture2D(input_frame, texture_uv);
}
