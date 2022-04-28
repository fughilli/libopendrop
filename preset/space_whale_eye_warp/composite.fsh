#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform sampler2D input;
uniform bool input_enable;

varying vec2 screen_uv;

vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

void main() {
  vec2 tex_uv = screen_to_tex(screen_uv);
  vec4 input_color = input_enable ? texture2D(input, tex_uv) : vec4(0, 0, 0, 0);
  gl_FragColor =
      mix(texture2D(render_target, tex_uv), input_color, input_color.w);
}
