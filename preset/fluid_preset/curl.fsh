#version 120

precision mediump float;
precision mediump sampler2D;

varying vec2 l_uv;
varying vec2 r_uv;
varying vec2 u_uv;
varying vec2 d_uv;
uniform sampler2D velocity;

void main() {
  float l = texture2D(velocity, l_uv).y;
  float r = texture2D(velocity, r_uv).y;
  float u = texture2D(velocity, u_uv).x;
  float d = texture2D(velocity, d_uv).x;
  // Positive vorticity is counter-clockwise.
  float vorticity = ((r - l) + (d - u)) / 2.0;
  gl_FragColor = vec4(vorticity, 0.0, 0.0, 1.0);
}
