#version 120

varying vec2 screen_uv;

uniform sampler2D last_frame;
uniform ivec2 last_frame_size;
uniform float energy;
uniform float power;
uniform float framerate_scale;

// Converts a normalized screen coordinate to a normalized texture coordinate.
vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

// Zooms a screen UV coordinate by displacing it along its axis.
vec2 zoom(vec2 screen_uv, float zoom) {
  return screen_uv * (1.0 + (zoom - 1.0) * framerate_scale);
}

// Returns the product of two sinusoids with the given coefficients of `arg`.
// Choosing the coefficients such that the number of decimal places required to
// represent their ratio is maximized will give a greater appearance of
// "randomness".
float cos_product(float coeff_a, float coeff_b, float arg) {
  return cos(coeff_a * arg) * cos(coeff_b * arg);
}

vec4 plus_sample(sampler2D texture, ivec2 texture_size, vec2 uv,
                 float support_pixels) {
  vec2 offset = support_pixels / vec2(texture_size);
  return (texture2D(last_frame, uv + vec2(offset.x, 0)) +
          texture2D(last_frame, uv + vec2(-offset.x, 0)) +
          texture2D(last_frame, uv + vec2(0, offset.y)) +
          texture2D(last_frame, uv + vec2(0, -offset.y))) /
         4.0f;
}

const float kMaxBrighten = 1.01;
const float kBrightenSwingMagnitude = 0.3;
const float kCosProductCoeffA = 23.456;
const float kCosProductCoeffB = 9.123;

void main() {
  vec2 texture_uv = vec2(0.0, 0.0);

  // Zoom the texture by an amount equal to a scaled sinusoid of the energy
  // times the power. This has the effect of the accumulated image zooming
  // inwards and then outwards in a periodic fashion, with the period of the
  // alternation being inversely proportional to the current intensity of the
  // audio. We multiply in the power such that instantaneous changes in the
  // audio cause more immediately perceptible "jumps" in the zoom effect.
  texture_uv = zoom(screen_uv, cos_product(34, 15, energy) * 0.3 * power + 1.0);

  texture_uv = screen_to_tex(texture_uv);

  // Mix the fragment color with the previously sampled color. Multiply the
  // sampled result by a value less than unity such that the energy input by the
  // drawn GL primitives dissipates over time.
  vec4 sample = plus_sample(last_frame, last_frame_size, texture_uv, 1.0f);
  float brighten_darken_coefficient =
      ((kMaxBrighten - kBrightenSwingMagnitude) +
       cos_product(20, 9, energy) * kBrightenSwingMagnitude);
  gl_FragColor = clamp(gl_Color * (1 - length(sample.rgb)) +
                           sample * brighten_darken_coefficient,
                       0, 1);
  if (length(gl_FragColor) < 0.1) {
    gl_FragColor = gl_FragColor * gl_FragColor;
  }
}
