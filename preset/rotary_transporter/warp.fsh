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

// Zooms a screen UV coordinate by displacing it along its axis.
vec2 zoom_towards(vec2 screen_uv, float zoom, vec2 target) {
  return (screen_uv - target) * zoom + target;
}

vec2 unit_at_angle(float angle) { return vec2(cos(angle), sin(angle)); }

// Returns the product of two sinusoids with the given coefficients of `arg`.
// Choosing the coefficients such that the number of decimal places required to
// represent their ratio is maximized will give a greater appearance of
// "randomness".
float sin_product(float coeff_a, float coeff_b, float arg) {
  return sin(coeff_a * arg) * sin(coeff_b * arg);
}

void main() {
  vec2 texture_uv = vec2(0.0, 0.0);

  // Zoom the texture by an amount equal to a scaled sinusoid of the energy
  // times the power. This has the effect of the accumulated image zooming
  // inwards and then outwards in a periodic fashion, with the period of the
  // alternation being inversely proportional to the current intensity of the
  // audio. We multiply in the power such that instantaneous changes in the
  // audio cause more immediately perceptible "jumps" in the zoom effect.
  texture_uv = zoom_towards(screen_uv, sin(energy * 20) * 0.3 * power + 1.1,
                            unit_at_angle(energy * 30));

  // Rotate the texture by a pseudo-random amount that varies by `energy`.
  // Additionally multiply in `power` for the same reason as above.
  texture_uv =
      rotate(texture_uv, sin_product(100 / 7.334, 10 / 9.225, energy) * power);

  texture_uv = screen_to_tex(texture_uv);

  // Mix the fragment color with the previously sampled color. Multiply the
  // sampled result by a value less than unity such that the energy input by the
  // drawn GL primitives dissipates over time.
  gl_FragColor = gl_Color * 0.5 + texture2D(last_frame, texture_uv) * 0.95;
}
