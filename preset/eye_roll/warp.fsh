#version 120

varying vec2 screen_uv;

uniform sampler2D last_frame;
uniform ivec2 last_frame_size;
uniform float energy;
uniform float power;

// Rotates a screen UV coordinate around the origin by `angle`.
vec2 rotate(vec2 screen_uv, float angle) {
  float c = cos(angle);
  float s = sin(angle);

  return vec2(c * screen_uv.x - s * screen_uv.y,
              s * screen_uv.x + c * screen_uv.y);
}

// Converts a normalized screen coordinate to a normalized texture coordinate.
vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

// Zooms a screen UV coordinate by displacing it along its axis.
vec2 zoom(vec2 screen_uv, float zoom) { return screen_uv * zoom; }

// Returns the product of two sinusoids with the given coefficients of `arg`.
// Choosing the coefficients such that the number of decimal places required to
// represent their ratio is maximized will give a greater appearance of
// "randomness".
float sin_product(float coeff_a, float coeff_b, float arg) {
  return sin(coeff_a * arg) * sin(coeff_b * arg);
}

const float kFilterKernel[9] =
    float[](0.0, 0.1, 0.0, 0.1, 0.6, 0.1, 0.0, 0.1, 0.0);

float map(float val, float in_low, float in_high, float out_low,
          float out_high) {
  return (val - in_low) / (in_high - in_low) * (out_high - out_low) + out_low;
}

vec3 hueShift(vec3 color, float hue)
{
const vec3 k = vec3(0.57735, 0.57735, 0.57735);
float cosAngle = cos(hue);
return vec3(color * cosAngle + cross(k, color) * sin(hue) + k * dot(k, color) * (1.0 - cosAngle));
}

vec4 texture_uv_with_color_sample(sampler2D tex, float p1, float p2, float p3,
                                  vec4 color) {
  float len = length(color);
  float x_offset = cos(screen_uv.y * 30 + p1) / p2;
  float y_offset = (p3 + 1 + cos(screen_uv.x * 10) / 2) / 100;
  vec2 texture_uv = screen_uv - vec2(x_offset, y_offset);
  texture_uv = screen_to_tex(texture_uv);
  if (len <= 0) {
    return texture2D(tex, texture_uv);
  }

  vec4 accum = vec4(0, 0, 0, 0);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      accum += texture2D(tex, texture_uv + vec2(map(i, 0, 2, -1, 1),
                                                map(j, 0, 2, -1, 1)) *
                                               len / 1000) *
               kFilterKernel[j * 3 + i];
    }
  }
  return accum;
}

void main() {
  float progression_term_1 = energy * 20 + power * 10;
  float progression_term_2 = power + 100;
  float progression_term_3 = energy / 10 + power;

  vec4 color = texture_uv_with_color_sample(
      last_frame, progression_term_1, progression_term_2, progression_term_3,
      vec4(0, 0, 0, 0));
  vec4 new_color = texture_uv_with_color_sample(last_frame, progression_term_1,
                                                progression_term_2,
                                                progression_term_3, color);

  vec4 hue_shift = vec4(hueShift(new_color.xyz, 2 * 3.1415926 / 3.000), 1);

  gl_FragColor = hue_shift * 0.975;//0.975;
}
