#version 120

varying vec2 texture_uv;

uniform sampler2D velocity;
uniform sampler2D source;
uniform ivec2 size;
uniform float dt;
uniform float dissipation;

vec4 bilerp(sampler2D sam, vec2 uv, vec2 tsize) {
  vec2 st = uv / tsize - 0.5;

  vec2 iuv = floor(st);
  vec2 fuv = fract(st);

  vec4 a = texture2D(sam, (iuv + vec2(0.5, 0.5)) * tsize);
  vec4 b = texture2D(sam, (iuv + vec2(1.5, 0.5)) * tsize);
  vec4 c = texture2D(sam, (iuv + vec2(0.5, 1.5)) * tsize);
  vec4 d = texture2D(sam, (iuv + vec2(1.5, 1.5)) * tsize);

  return mix(mix(a, b, fuv.x), mix(c, d, fuv.x), fuv.y);
}

void main() {
//#if 0
  vec2 tsize = 1.0 / size;
  vec2 coord = texture_uv - dt * bilerp(velocity, texture_uv, tsize).xy * tsize;
  vec4 result = bilerp(source, coord, tsize);
//#else
//  vec2 tsize = 1.0 / size;
//  vec2 coord = texture_uv - dt * texture2D(velocity, texture_uv).xy * tsize;
//  vec4 result = texture2D(source, coord);
//#endif
  float decay = 1.0 + dissipation * dt;
  gl_FragColor = vec4(result.xyz / decay, 1.0);
}
