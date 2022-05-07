#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;

uniform sampler2D last_frame;
uniform ivec2 last_frame_size;

const bool kScreenToTex = true;
const bool kClamp = false;
const bool kEnableWarp = true;
const int kNumSamples = 4;
const float kEnergyMultiplier = 100;
const int kMaxDivisions = 6;
const int kMinDivisions = 1;

uniform float blur_distance;
uniform float warp_zoom_coeff;
uniform float warp_rot_coeff;
uniform float sample_rot_coeff;
uniform float sample_scale_coeff;
uniform float color_coeff;
uniform vec2 blur_offset;
uniform int num_divisions;

void main() {
  vec2 texture_uv = vec2(0.0, 0.0);

  texture_uv = rotate(
      zoom(uv_warp_kaleidoscope(screen_uv, num_divisions), warp_zoom_coeff),
      warp_rot_coeff);
  vec2 sampling_displacement =
      unit_at_angle(sample_rot_coeff) * sample_scale_coeff;

  texture_uv += sampling_displacement;

  texture_uv = screen_to_tex(texture_uv);

  vec4 sampled_color =
      (texture2D(last_frame,
                 texture_uv + vec2(blur_offset.x, 0.) * blur_distance) +
       texture2D(last_frame,
                 texture_uv - vec2(blur_offset.x, 0.) * blur_distance) +
       texture2D(last_frame,
                 texture_uv + vec2(0., blur_offset.y) * blur_distance) +
       texture2D(last_frame,
                 texture_uv - vec2(0., blur_offset.y) * blur_distance)) *
      (1.0 / kNumSamples);

  gl_FragColor = sampled_color * color_coeff + gl_Color;
}
