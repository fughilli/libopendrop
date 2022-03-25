#version 120

varying vec2 screen_uv;

void main() {
  screen_uv = ftransform().xy;
  gl_Position = ftransform();
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
}
