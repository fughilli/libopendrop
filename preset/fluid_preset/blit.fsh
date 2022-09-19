#version 120

precision highp float;
precision highp sampler2D;

varying vec2 texture_uv;
uniform sampler2D source_texture;

void main() {
  gl_FragColor = vec4(texture2D(source_texture, texture_uv).xyz / 10.0 + 0.5, 1.0);
}
