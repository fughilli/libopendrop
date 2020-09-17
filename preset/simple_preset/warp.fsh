#version 120

varying vec2 screen_uv;

uniform sampler2D last_frame;
uniform ivec2 last_frame_size;
uniform float energy;
uniform float power;

const bool kScreenToTex = true;
const bool kClamp = false;
const bool kEnableWarp = true;
const int kNumSamples = 4;

vec2 rotate(vec2 screen_uv, float angle) {
  float c = cos(angle);
  float s = sin(angle);

  return vec2(c * screen_uv.x - s * screen_uv.y,
              s * screen_uv.x + c * screen_uv.y);
}

float rect(float x, float period) {
  float interp = mod(x, period);
  return (interp > period / 2.0) ? 1.0 : -1.0;
}

vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

vec2 zoom(vec2 screen_uv, float zoom) { return screen_uv * zoom; }

void main() {
  vec2 offset = 1. / last_frame_size;
  float mod_a =
      sin(energy * 0.313 * 3.1415926) + sin(10 * energy / 9.225) * power / 10;
  float mod_b =
      sin(energy * 0.0782 * 3.1415926) + sin(10 * energy / 7.932) * power / 10;

  vec2 texture_uv = vec2(0.0, 0.0);

  if (kEnableWarp) {
    // Compute the base texture sampling oordinate as a rotated screen-space uv
    // around the origin, with theta determined as a sinusoid of the input
    // energy plus the power. Zoom by a similar coefficient.
    texture_uv = rotate(
        zoom(screen_uv, mod_a / 20 * power + 1.),
        sin(100 * energy / 7.334) * sin(10 * energy / 9.225) * mod_b * power);
    // Compute a sampling displacement which is equivalent to the unit vector
    // along x scaled by a small value which is sinusoidally oscillating across
    // the y axis, all rotated by an angle which is a linear function of the
    // input signal power and accumulated energy.
    float direction_change = rect(energy, 1.);
    vec2 sampling_displacement =
        rotate(vec2((1. + sin(direction_change * energy / 20)) / 30, 0),
               energy / 10 + power);

    texture_uv += sampling_displacement;
  } else {
    texture_uv = zoom(screen_uv, mod_a / 20 * power + 1.);
  }
  if (kScreenToTex) {
    texture_uv = screen_to_tex(texture_uv);
  }

  if (kClamp) {
    texture_uv = clamp(texture_uv, (kScreenToTex ? 0. : -1.), 1.);
  }

  float blur_distance = max(0.0, 1 - power * 1);

  vec4 out_color =
      gl_Color * 0.5 +
      (texture2D(last_frame, texture_uv + vec2(offset.x, 0.) * blur_distance) +
       texture2D(last_frame, texture_uv - vec2(offset.x, 0.) * blur_distance) +
       texture2D(last_frame, texture_uv + vec2(0., offset.y) * blur_distance) +
       texture2D(last_frame, texture_uv - vec2(0., offset.y) * blur_distance)) *
          (1.0 / kNumSamples);

  // Modulate the summation weights such that they oscillate
  // back and forth across the +1 boundary. This causes the
  // image to "brighten" during some time, and "darken" during
  // the rest. Also, we modulate by `sin(theta * coeff)` (theta
  // being equal to `atan2(pixel.y, pixel.x)`) so as to achieve
  // a "pinwheel" effect, where certain petals of the pinwheel
  // have a "brightening" effect, and the other pinwheels have
  // a "darkening" effect.
  float brighten_darken_weight =
      (sin(atan(screen_uv.y, screen_uv.x) * 3 + energy) / 10 + 0.94);

  gl_FragColor = out_color * brighten_darken_weight;
}
