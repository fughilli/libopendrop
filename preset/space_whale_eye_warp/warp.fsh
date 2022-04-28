#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;

uniform sampler2D last_frame;
uniform sampler2D input;
uniform ivec2 frame_size;
uniform vec3 zoom_vec;
uniform float energy;
uniform float power;

void main() {
  vec2 texture_uv = vec2(0.0, 0.0);

  // Zoom the texture by an amount equal to a scaled sinusoid of the energy
  // times the power. This has the effect of the accumulated image zooming
  // inwards and then outwards in a periodic fashion, with the period of the
  // alternation being inversely proportional to the current intensity of the
  // audio. We multiply in the power such that instantaneous changes in the
  // audio cause more immediately perceptible "jumps" in the zoom effect.
  float zoom_coeff = 1.0 + zoom_vec.z / 10;
  texture_uv = zoom_towards(screen_uv, zoom_coeff, -zoom_vec.xy);

  // Rotate the texture by a pseudo-random amount that varies by `energy`.
  // Additionally multiply in `power` for the same reason as above.
  texture_uv =
      rotate(texture_uv, sin_product(100 / 7.334, 10 / 9.225, energy) * power);

  texture_uv = screen_to_tex(texture_uv);

  // Mix the fragment color with the previously sampled color. Multiply the
  // sampled result by a value less than unity such that the energy input by the
  // drawn GL primitives dissipates over time.
  vec4 input_color = texture2D(input, screen_to_tex(screen_uv));
  vec4 last_color =
      blur_sample_texture(last_frame, texture_uv, frame_size, 0.5);
  gl_FragColor =
      mix(input_color, mix(vec4(1, 1, 0, 1), last_color, last_color.w),
          1 - input_color.w);
}
