#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;
uniform float power;
uniform float normalized_energy;

varying vec2 screen_uv;

vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

void main() {
  vec2 tex_uv = screen_to_tex(screen_uv);
  // Offset sampling to implement a Sobel filter kernel. The support of the
  // kernel is modulated by the instantaneous power.
  float offset_size_pixels = (3. + power * 10);

  // Compute the offset in normalized texture coordinates.
  vec2 offset = offset_size_pixels / render_target_size;

  // Sample the texture at 3 locations:
  //   1. Corresponding location for current fragment
  //   2. Corresponding location for current fragment offset in X
  //   3. Corresponding location for current fragment offset in Y
  //
  // Combine the samples with kernel [1., -0.2, -0.2] such that edges are mildly
  // accentuated.
  gl_FragColor = gl_Color + texture2D(render_target, tex_uv) * 1.2 +
                 (texture2D(render_target, tex_uv + vec2(offset.x, 0.)) +
                  texture2D(render_target, tex_uv + vec2(0., offset.y))) *
                     -0.1
      // TODO: Produce a more interesting effect for the Sobel
      // filter coefficient
      ;
}
