#version 120

#include "preset/common/math.shh"

uniform vec2 pole;
varying vec2 screen_uv;

uniform sampler2D last_frame;

uniform float rotate_coeff;
uniform float phase_x_coeff;
uniform float phase_y_coeff;
uniform float ripple_hue;
uniform float min_value_coeff;
uniform float fisheye_coeff;

void main() {
  // float hue = 1.0 / (kEpsilon + length(screen_uv - pole));
  vec2 effect_uv = uv_warp_fisheye(screen_uv, fisheye_coeff);
  vec2 coord = effect_uv + pole;
  vec2 coord2 = effect_uv - pole;
  vec2 phase = vec2(
      sin_product(coord2.x, coord2.y, 12.345 * (1 + phase_x_coeff)),
      sin_product(coord2.x * 3, coord2.y * 4, 11.11 * (1 + phase_y_coeff)));
  coord = rotate(coord, 4 * kPi * rotate_coeff);
  float color_value =
      1.0 / limit_min_magnitude(
                sin((coord.x) * 20 + phase.x) + sin((coord.y) * 20 + phase.y),
                min_value_coeff, kLimitMinMagnitudePolicy_Rolloff);
  gl_FragColor =
      vec4(hsv_to_rgb(vec3(ripple_hue, 1.0, mod(color_value, 1))), 1.0);
}
