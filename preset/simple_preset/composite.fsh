#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform float power;
uniform float alpha;

varying vec2 screen_uv;

vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

void main() {
  vec2 tex_uv = screen_to_tex(screen_uv);
  vec2 offset = (3. + power * 10) / render_target_size;
  gl_FragColor = (texture2D(render_target, tex_uv) * 1. -
                  (texture2D(render_target, tex_uv + vec2(offset.x, 0.)) +
                   texture2D(render_target, tex_uv + vec2(0., offset.y))) *
                      0.2) *
                 alpha;
}
