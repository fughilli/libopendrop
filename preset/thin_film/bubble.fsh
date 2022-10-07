#version 120

#include "preset/common/math.shh"

uniform vec2 pole;
varying vec2 screen_uv;

uniform sampler2D last_frame;

uniform float rotate_coeff;
uniform float phase_x_coeff;
uniform float phase_y_coeff;
uniform float ripple_hue;
uniform float shift_hue_coeff;
uniform float min_value_coeff;
uniform float fisheye_coeff;
uniform float folds_coeff;
uniform float force_mix_coeff;
uniform float mix_prescale_coeff;
uniform bool invert_screen;
uniform bool invert_hue;
uniform bool invert_coords;
uniform bool swap_coords;
uniform bool swap_recursive_coords;
uniform bool invert_recursive_coords;

void main() {
  vec2 in_screen_uv =
      uv_warp_swap(uv_warp_invert(screen_uv, invert_coords), swap_coords);
  vec2 effect_uv = uv_warp_fisheye(in_screen_uv, fisheye_coeff);
  vec2 coord = rotate(effect_uv, 4 * kPi * rotate_coeff) + pole;
  vec2 coord2 = rotate(effect_uv, 4 * kPi * rotate_coeff) - pole;
  vec2 phase = vec2(
      sin_product(coord2.x, coord2.y, 12.345 * (1 + phase_x_coeff)),
      sin_product(coord2.x * 3, coord2.y * 4, 1.11 * (1 + 10 * phase_y_coeff)));
  float color_value =
      1.0 / limit_min_magnitude(
                sin((coord.x) * 20 + phase.x) + sin((coord.y) * 20 + phase.y),
                min_value_coeff, kLimitMinMagnitudePolicy_Rolloff);

  float input_hue = ripple_hue;
  if (invert_hue) {
    input_hue += 0.5;
  }

  vec4 color = vec4(hsv_to_rgb(vec3(input_hue, 1.0, mod(color_value, 1))), 1.0);
  vec2 folded_uv =
      uv_warp_fold(in_screen_uv * 2, vec2(0, 0), vec2(1, 1) * folds_coeff);
  vec2 inverted_uv = uv_warp_invert(folded_uv, invert_recursive_coords);
  vec2 swapped_uv = uv_warp_swap(inverted_uv, swap_recursive_coords);
  vec4 last_frame_color = texture2D(last_frame, screen_to_tex(swapped_uv));
  // Give control over how bright the forcibly-mixed color is.
  last_frame_color = vec4(min(last_frame_color.rgb * mix_prescale_coeff, 1), 1);
  vec4 out_color = mix(color, shift_hue(last_frame_color, shift_hue_coeff),
                       max(length(color.rgb), force_mix_coeff));

  if (invert_screen) {
    out_color.xyz = vec3(1, 1, 1) - out_color.xyz;
  }
  gl_FragColor = out_color;
}
