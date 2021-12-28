#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform float energy;

varying vec2 screen_uv;
varying vec2 texture_uv;
varying vec3 normal;

uniform bool black;

uniform vec4 light_color_a;
uniform vec4 light_color_b;

void main() {
  vec3 light_direction = vec3(cos(energy), sin(energy), -0.6);
  gl_FragColor =
      black ? vec4(0, 0, 0, 1)
            : (texture2D(render_target, texture_uv) * 0.5 +
               mix(light_color_a, light_color_b,
                   dot(normalize(normal), normalize(light_direction))) *
                   0.5);
}
