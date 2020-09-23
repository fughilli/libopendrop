#version 120

varying vec2 screen_uv;

const float PI = 3.1415926;

uniform sampler2D last_frame;
uniform ivec2 last_frame_size;
uniform float energy;
uniform float power;

const bool kScreenToTex = true;
const bool kClamp = false;
const bool kEnableWarp = true;
const int kNumSamples = 4;
const float kEnergyMultiplier = 100;
const int kMaxDivisions = 6;
const int kMinDivisions = 1;

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
  int num_divisions =
      int((sin(energy * 3) + 1.) / 2. * (kMaxDivisions - kMinDivisions)) +
      kMinDivisions;
  float energy_scaled = energy * kEnergyMultiplier;

  vec2 offset = 1. / last_frame_size;
  float mod_a = sin(energy_scaled * 0.313 * PI) +
                sin(10 * energy_scaled / 9.225) * power / 10;
  float mod_b = sin(energy_scaled * 0.0782 * PI) +
                sin(10 * energy_scaled / 7.932) * power / 10;

  vec2 attractor = vec2(0., 0.);
  // vec2(cos(energy_scaled * 10) / 10 + cos(energy_scaled * 20) / 4,
  //     sin(energy_scaled * 10) / 10 + cos(energy_scaled * 30) / 4);
  float angle = atan(screen_uv.y + attractor.y, screen_uv.x + attractor.x) +
                sin(energy_scaled * 3.12) * sin(energy_scaled * 7.521) * 0.3;

  float mod_angle = mod(angle, PI / num_divisions);
  if (mod_angle > PI / (num_divisions * 2)) {
    mod_angle = (PI / num_divisions) - mod_angle;
  }

  vec2 sampling_screen_uv =
      rotate(vec2(length(screen_uv), 0),
             mod_angle +
                 sin(energy_scaled * 7.1) * sin(energy_scaled * 2.391) * 0.3) +
      attractor;
  vec2 texture_uv = vec2(0.0, 0.0);

  if (kEnableWarp) {
    // Compute the base texture sampling oordinate as a rotated screen-space uv
    // around the origin, with theta determined as a sinusoid of the input
    // energy_scaled plus the power. Zoom by a similar coefficient.
    texture_uv =
        rotate(zoom(sampling_screen_uv, mod_a / 20 * power / 100 + 1.0),
               sin(100 * energy_scaled / 7.334) *
                   sin(10 * energy_scaled / 9.225) * mod_b * power / 100) +
        attractor;
    // Compute a sampling displacement which is equivalent to the unit vector
    // along x scaled by a small value which is sinusoidally oscillating across
    // the y axis, all rotated by an angle which is a linear function of the
    // input signal power and accumulated energy_scaled.
    float direction_change = rect(energy_scaled, 1.);
    vec2 sampling_displacement =
        rotate(vec2((1. + sin(direction_change * energy_scaled / 20)) / 30, 0),
               energy_scaled / 10 + power);

    texture_uv += sampling_displacement;
  } else {
    texture_uv = zoom(screen_uv, mod_a / 20 * power / 100 + 1.);
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
      (sin(((screen_uv.y + energy_scaled) * screen_uv.x) * 0.2 +
           energy_scaled) /
           10 +
       0.99);

  gl_FragColor = out_color * brighten_darken_weight;
}
