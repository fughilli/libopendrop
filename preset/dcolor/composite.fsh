#version 120

#include "preset/common/complex.shh"
#include "preset/common/math.shh"

uniform float aspect_ratio;
uniform vec2 zero_loc;
uniform vec2 pole_loc;
uniform vec2 zero_loc2;
uniform vec2 pole_loc2;
uniform vec2 zero_loc3;
uniform vec2 pole_loc3;
uniform vec2 zero_loc4;
uniform vec2 pole_loc4;

varying vec2 screen_uv;
varying vec2 texture_uv;

vec4 dcolor(vec2 z) {
  float r = log(1 + length(z));
  float s = (1 + abs(sin(2 * kPi * r))) / 2;
  float v = (1 + abs(cos(2 * kPi * r))) / 2;
  float h = mod(atan(z.y, z.x), 2 * kPi) / (2 * kPi);

  return vec4(hsv_to_rgb(vec3(h, s, v)), 1.0f);
}

void main() {
  vec2 x = screen_uv;
  x.x /= aspect_ratio;

  vec2 top_prod =
      cmul((x - zero_loc),
           cmul((x - zero_loc2), cmul((x - zero_loc3), (x - zero_loc4))));
  vec2 bottom_prod =
      cmul((x - pole_loc),
           cmul((x - pole_loc2), cmul((x - pole_loc3), (x - pole_loc4))));
  vec2 z = cdiv(top_prod, bottom_prod);

  gl_FragColor = dcolor(z);
}
