#version 120

varying vec2 screen_uv;
varying vec2 texture_uv;
varying vec3 normal;
uniform mat4 model_transform;

void main() {
  vec4 transformed = model_transform * gl_Vertex;
  screen_uv = transformed.xy;
  texture_uv = gl_MultiTexCoord0.xy;
  normal = (model_transform * vec4(gl_Normal, 0)).xyz;
  gl_Position = transformed;
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
}
