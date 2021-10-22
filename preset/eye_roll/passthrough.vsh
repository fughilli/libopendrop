#version 120

varying vec2 screen_uv;
uniform mat4 model_transform;

void main() {
  vec4 transformed = model_transform * gl_Vertex;
  screen_uv = transformed.xy;
  gl_Position = transformed;
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
}
