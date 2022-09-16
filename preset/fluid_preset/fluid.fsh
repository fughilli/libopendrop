#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;

uniform float dt;
uniform sampler2D input_frame;
uniform ivec2 input_frame_size;
uniform sampler2D color_frame;

void main() {
  vec2 texture_uv = screen_to_tex(screen_uv);

  vec4 cell = texture2D(input_frame, texture_uv);

  vec2 velocity = cell.xy;
  vec2 advect_from_pos =
      texture_uv - dt * vec2(velocity.x / input_frame_size.x,
                             velocity.y / input_frame_size.y);

  gl_FragColor = texture2D(color_frame, advect_from_pos);
}
