#version 120

#include "libopendrop/preset/common/math.shh"

varying vec2 screen_uv;

uniform sampler2D last_frame;
uniform ivec2 last_frame_size;
uniform float energy;
uniform float power;

const float kFilterKernel[9] =
    float[](0.2, -0.1, 0.2, -0.1, 0.65, -0.1, 0.2, -0.1, 0.2);


vec4 texture_uv_with_color_sample(sampler2D tex, float p1, float p2, float p3,
                                  vec4 color) {
  float len = color.x;
  float x_offset =
      cos(screen_uv.y * sin_product_range(7.123, 3.459, 2, 30, energy) + p1) /
      p2;
  float y_offset =
      (p3 + 1 +
       cos(screen_uv.x * sin_product_range(9.111, 6.591, 0.5, 10, energy)) /
           2) /
      100;
  vec2 texture_uv = screen_uv - vec2(x_offset, y_offset);
  texture_uv = screen_to_tex(texture_uv);
  if (len <= 0) {
    return texture2D(tex, texture_uv);
  }

  vec4 accum = vec4(0, 0, 0, 0);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      accum +=
          texture2D(tex,
                    texture_uv +
                        rotate(vec2(map(i, 0, 2, -1, 1), map(j, 0, 2, -1, 1)),
                               len * sin_product_range(4.293, 3.847, 0.2, 10,
                                                       energy)) *
                            len / 200) *
          kFilterKernel[j * 3 + i];
    }
  }
  return accum;
}

void main() {
  float progression_term_1 = energy * 2 + power * 1;
  float progression_term_2 = power + 50;
  float progression_term_3 = energy / 20 + power;

  vec4 color = texture_uv_with_color_sample(
      last_frame, progression_term_1, progression_term_2, progression_term_3,
      vec4(0, 0, 0, 0));
  vec4 new_color = texture_uv_with_color_sample(last_frame, progression_term_1,
                                                progression_term_2,
                                                progression_term_3, color);

  float len = length(color);

  vec4 hue_shift = shift_hue(new_color, len / 5);

  gl_FragColor = hue_shift * 0.955;  // 0.975;
}
