#version 120

#include "preset/common/math.shh"

varying vec2 screen_uv;
varying vec2 texture_uv;
varying vec3 normal;

uniform float phase;

const float kDutyCycle = 0.5;
const float kStride = 0.1;
const vec2 kDirection = vec2(1.0, 0.0);

const vec4 kOnColor = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 kOffColor = vec4(0.0, 0.0, 0.0, 1.0);

void main() {
  float arg = dot(screen_uv, kDirection) + phase;
  bool on = mod(arg, kStride) < (kStride * kDutyCycle);
  gl_FragColor = on ? kOnColor : kOffColor;
}
