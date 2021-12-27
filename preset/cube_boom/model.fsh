#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform float power;

varying vec2 screen_uv;
varying vec2 texture_uv;
varying vec3 normal;

const vec3 kLightDirection = vec3(-0.5, -0.5, -0.2);

void main() {
  const vec3 light_norm = normalize(kLightDirection);
  vec3 norm = normalize(normal);
  gl_FragColor = texture2D(render_target, texture_uv) *
                 (1 - dot(normalize(normal), normalize(kLightDirection)));
}
