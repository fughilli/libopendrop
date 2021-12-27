#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform float power;

varying vec2 screen_uv;
varying vec2 texture_uv;
varying vec3 normal;

uniform bool black;

const vec4 kLightColor1 = vec4(1, 0, 0, 1);
const vec4 kLightColor2 = vec4(0, 0, 1, 1);
const vec3 kLightDirection = vec3(-0.5, -0.5, -0.4);

void main() {
  gl_FragColor =
      black ? vec4(0, 0, 0, 1)
            : (texture2D(render_target, texture_uv) * 0.5 +
               mix(kLightColor1, kLightColor2,
                   dot(normalize(normal), normalize(kLightDirection))) *
                   0.5);
}
