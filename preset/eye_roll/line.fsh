#version 120

varying vec2 screen_uv;

uniform float energy;
uniform float power;

const float kPi = 3.1415926;

vec3 HsvToRgb(vec3 hsv) {
  // C = V x Sv
  float c = clamp(hsv.y, 0., 1.) * clamp(hsv.z, 0., 1.);

  // H' = H/60deg
  // X = chroma x (1 - |H' mod 2 - 1|)
  float x = c * (1. - abs(mod(hsv.x * 6., 2.) - 1.));

  int hsv_case = int(mod(int(hsv.x * 6), 6));
  if (hsv_case == 0) return vec3(c, x, 0.);
  if (hsv_case == 1) return vec3(x, c, 0.);
  if (hsv_case == 2) return vec3(0., c, x);
  if (hsv_case == 3) return vec3(0., x, c);
  if (hsv_case == 4) return vec3(x, 0., c);
  if (hsv_case == 5) return vec3(c, 0., x);
  return vec3(0., 0., 0.);
}

void main() {
  float color_angle =
      mod((1 + screen_uv.x * cos(energy)) / 2 + energy * 10 + power, 1);
  gl_FragColor = vec4(HsvToRgb(vec3(color_angle, 1, 1)), 1);
}
