#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;

uniform sampler2D input_frame;
uniform ivec2 input_frame_size;

void main() {
  vec2 texture_uv = screen_to_tex(screen_uv);

  vec4 cell = texture2D(input_frame, texture_uv);
  vec4 cell_left =
      texture2D(input_frame, texture_uv - vec2(1.0 / input_frame_size.x, 0.0)) -
      vec4(0.5, 0.5, 0, 0);
  vec4 cell_above =
      texture2D(input_frame, texture_uv - vec2(0.0, 1.0 / input_frame_size.y)) -
      vec4(0.5, 0.5, 0, 0);

  float pressure = cell.z + ((cell_left.x - cell.x) + ((cell_above.y - cell.y))) / 2;
  vec2 velocity = vec2(cell.z - cell_left.z, (cell.z - cell_above.z)) / 2;

  gl_FragColor = vec4(velocity, pressure, 1.0);
}
