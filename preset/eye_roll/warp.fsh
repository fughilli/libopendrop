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

void main() {
  vec2 texture_uv =
      screen_uv -
      vec2(cos(screen_uv.y * 10 + energy * 20 + power * 10) / (power + 100),
           (energy / 10 + power + 1) / last_frame_size.y);
  texture_uv = screen_to_tex(texture_uv);

  gl_FragColor = texture2D(last_frame, texture_uv) * 0.975;
}
