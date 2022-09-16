#version 120

varying vec2 screen_uv;

uniform sampler2D velocity;
uniform sampler2D source;
uniform vec2 size;
uniform float dt;
uniform float dissipation;

void main() {
  vec2 texture_uv = screen_to_tex(screen_uv);
  vec2 coord = texture_uv - dt * texture2D(velocity, texture_uv).xy / size;
  vec4 result = texture2D(source, coord);
  float decay = 1.0 + dissipation * dt;
  gl_FragColor = result / decay;
}
