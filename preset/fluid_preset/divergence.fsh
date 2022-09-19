#version 120

precision mediump float;
precision mediump sampler2D;

varying vec2 texture_uv;
varying vec2 l_uv;
varying vec2 r_uv;
varying vec2 u_uv;
varying vec2 d_uv;
uniform sampler2D velocity;

void main() {
  float l = texture2D(velocity, l_uv).x;
  float r = texture2D(velocity, r_uv).x;
  float u = texture2D(velocity, u_uv).y;
  float d = texture2D(velocity, d_uv).y;
  vec2 c = texture2D(velocity, texture_uv).xy;

  if (l_uv.x < 0.0) {
    l = -c.x;
  }
  if (r_uv.x > 1.0) {
    r = -c.x;
  }
  if (u_uv.y > 1.0) {
    u = -c.y;
  }
  if (d_uv.y < 0.0) {
    d = -c.y;
  }

  float divergence = ((r - l) + (u - d)) / 2.0;
  gl_FragColor = vec4(divergence, 0.0, 0.0, 1.0);
}
