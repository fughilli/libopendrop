#version 120

varying vec2 screen_uv;

uniform float energy;
uniform vec2 center;
uniform float y_cutoff;

const float kPi = 3.1415926;

vec3 HsvToRgb(vec3 hsv) {
  // C = V x Sv
  float c = clamp(hsv.y, 0., 1.) * clamp(hsv.z, 0., 1.);

  // H' = H/60deg
  // X = chroma x (1 - |H' mod 2 - 1|)
  float x = c * (1. - abs(mod(hsv.x * 6., 2.) - 1.));

  switch (int(mod(int(hsv.x * 6), 6))) {
    case 0:
      return vec3(c, x, 0.);
    case 1:
      return vec3(x, c, 0.);
    case 2:
      return vec3(0., c, x);
    case 3:
      return vec3(0., x, c);
    case 4:
      return vec3(x, 0., c);
    case 5:
      return vec3(c, 0., x);
    default:
      return vec3(0., 0., 0.);
  }
}

void main() {
  vec2 displacement = screen_uv - center;
  float angle = atan(displacement.y, displacement.x);
  float color_angle = mod(angle / (2 * kPi) + energy, 1);
  if (screen_uv.y <= y_cutoff) {
    gl_FragColor = vec4(HsvToRgb(vec3(color_angle, 1, 1)), 1);
  } else {
    gl_FragColor = vec4(0, 0, 0, 1);
  }
}
