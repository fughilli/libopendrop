#version 120

varying vec2 screen_uv;

uniform sampler2D last_frame;
uniform ivec2 last_frame_size;
uniform float energy;
uniform float power;

const bool kScreenToTex = true;
const bool kClamp = false;
const bool kEnableWarp = true;

vec2 rotate(vec2 screen_uv, float angle) {
  float c = cos(angle);
  float s = sin(angle);

  return vec2(c * screen_uv.x - s * screen_uv.y,
              s * screen_uv.x + c * screen_uv.y);
}

vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

vec2 zoom(vec2 screen_uv, float zoom) { return screen_uv * zoom; }

void main() {
  vec2 offset = 1. / last_frame_size;
  float mod_a = sin(energy * 0.313 * 3.1415926) + power / 10;
  float mod_b = sin(energy * 0.0782 * 3.1415926) + power / 10;
  vec2 texture_uv =
      kEnableWarp
          ? (rotate(zoom(screen_uv, mod_a / 20 * power + 1.), mod_b * power) +
             rotate(vec2((1. + sin(energy / 20)) / 30, 0), energy / 10 + power))
          : zoom(screen_uv, mod_a / 20 * power + 1.);
  if (kScreenToTex) {
    texture_uv = screen_to_tex(texture_uv);
  }

  if (kClamp) {
    texture_uv = clamp(texture_uv, (kScreenToTex ? 0. : -1.), 1.);
  }

  float blur_distance = 2 - power * 10;

  gl_FragColor =
      gl_Color * 0.5 +
      (texture2D(last_frame, texture_uv + vec2(offset.x, 0.) * blur_distance) +
       texture2D(last_frame, texture_uv - vec2(offset.x, 0.) * blur_distance) +
       texture2D(last_frame, texture_uv + vec2(0., offset.y) * blur_distance) +
       texture2D(last_frame, texture_uv - vec2(0., offset.y) * blur_distance)) *
          0.25 * (sin(atan(screen_uv.y, screen_uv.x) * 3 + energy) / 10 + 0.97);
}
