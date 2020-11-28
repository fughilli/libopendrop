#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform float power;
uniform float energy;
uniform float alpha;

varying vec2 screen_uv;

vec2 rotate(vec2 vec, float theta) {
  float s = sin(theta);
  float c = cos(theta);

  return vec2(vec.x * s - vec.y * c, vec.x * c + vec.y * s);
}

vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

void main() {
  vec2 screen_uv_p = rotate(screen_uv, energy * 5);

  vec2 tex_uv = screen_to_tex(screen_uv_p);
  gl_FragColor = texture2D(render_target, tex_uv);
}
