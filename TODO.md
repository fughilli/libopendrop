## TODO List

- Implement complex preset blending. Presets provide accessors for all shaders
  and generators, which can be mixed with other presets.
- Implement dynamic framerate scaling. Moving running windows between displays
  with different framerates should work correctly.
- Implement #include preprocessor directives for GLSL in presets.
- Runtime toolchain could be implemented as a standalone shell script which
  invokes clang to build a shared library of the preset, and then dynamically
  links it with the running libopendrop instance.
- Deploy to RPi and measure performance.
- Implement performance counters.
- Consolidate preset boilerplate and common rendering code into libraries.
- Implement live-updating preset workspace.
- Implement tool for rendering snippets of program execution with given shader
  programs.
- Implement primitves for calculating input values for presets with behavior
  that is consistent across framerates and resolutions. Implement unit tests to
  assert this behavior.
- Implement common subexpression eliminiation for filter pipelines (construct
  processing pipeline for a preset, reuse if another preset refers to it).
- Normalize global state signal accumulation by framerate.

- Fix audio buffer synchronization behavior. Mark the previous buffer contents
  as "consumed" at the end of frame processing, but don't actually remove the
  samples until the next interrupt to ensure that for high refresh rate displays
  we don't get an empty buffer (DrawFrame() happens twice with no audio
  interrupt).
