#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform float power;

varying vec2 screen_uv;
varying vec2 texture_uv;

void main() { gl_FragColor = texture2D(render_target, texture_uv); }
