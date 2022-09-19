#version 120

precision mediump float;
precision mediump sampler2D;

varying vec2 texture_uv;
varying vec2 l_uv;
varying vec2 r_uv;
varying vec2 u_uv;
varying vec2 d_uv;
uniform sampler2D pressure;
uniform sampler2D velocity;

void main() {
  float l = texture2D(pressure, l_uv).x;
  float r = texture2D(pressure, r_uv).x;
  float u = texture2D(pressure, u_uv).x;
  float d = texture2D(pressure, d_uv).x;
  vec2 velocity = texture2D(velocity, texture_uv).xy;
  velocity.xy -= vec2(r - l, u - d);
  gl_FragColor = vec4(velocity, 0.0, 1.0);
}
