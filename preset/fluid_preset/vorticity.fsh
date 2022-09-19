#version 120

#include "preset/common/math.shh"

precision highp float;
precision highp sampler2D;

varying vec2 texture_uv;
varying vec2 l_uv;
varying vec2 r_uv;
varying vec2 u_uv;
varying vec2 d_uv;
uniform sampler2D velocity;
uniform sampler2D curl;
uniform float curl_coeff;
uniform float dt;

void main() {
  float l = texture2D(curl, l_uv).x;
  float r = texture2D(curl, r_uv).x;
  float u = texture2D(curl, u_uv).x;
  float d = texture2D(curl, d_uv).x;
  float c = texture2D(curl, texture_uv).x;

  vec2 force = 0.5 * vec2(abs(u) - abs(d), abs(r) - abs(l));
  // Prevent division by zero with epsilon.
  force /= length(force) + kEpsilon;
  force *= curl_coeff * c;
  // Flip to texture coords.
  force.y *= -1.0;

  vec2 velocity = texture2D(velocity, texture_uv).xy;
  velocity += force * dt;
  velocity = min(max(velocity, -1000.0), 1000.0);
  gl_FragColor = vec4(velocity, 0.0, 1.0);
}
