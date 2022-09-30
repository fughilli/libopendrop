#version 120

#include "preset/common/math.shh"

uniform vec2 pole;
varying vec2 screen_uv;

void main() {
  // float hue = 1.0 / (kEpsilon + length(screen_uv - pole));
  vec2 coord = screen_uv + pole;
  vec2 coord2 = screen_uv - pole;
  vec2 phase = vec2(sin_product(coord2.x, coord2.y, 12.345),
                    sin_product(coord2.x * 3, coord2.y * 4, 11.11));
  float hue =
      1.0 / (sin((coord.x) * 20 + phase.x) + sin((coord.y) * 20) + phase.y);
  gl_FragColor = vec4(hsv_to_rgb(vec3(hue, 1.0, 1.0)), 1.0);
}
