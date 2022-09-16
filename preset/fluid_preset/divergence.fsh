#version 120

varying vec2 screen_uv;
uniform sampler2D velocity;
uniform vec2 size;

void main() {
  vec2 texture_uv = screen_to_tex(screen_uv);

  vec2 tex_size = vec2(1,1) / size;
  float lx = texture2D(velocity,texture_uv - vec2(tex_size.x, 0)).x;
  float rx = texture2D(velocity,texture_uv + vec2(tex_size.x, 0)).x;
  float ty = texture2D(velocity,texture_uv - vec2(0, tex_size.y)).y;
  float by = texture2D(velocity,texture_uv + vec2(0, tex_size.y)).y;

  vec2 c = texture2d(velocity, texture_uv).xy;
  if (
}
