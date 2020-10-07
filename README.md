# libopendrop

`libopendrop` is a visualizer library à la Milkdrop2 or ProjectM. However,
unlike both Milkdrop2 and ProjectM, it is implemented using modern C++17 with a
central focus on correctness, performance, and simplicity.

Preset authoring is not restricted by an arbitrary shader model; while the
underlying `libopendrop` engine is compatible with Milkdrop2 and ProjectM-style
presets, `libopendrop` is more flexible in that it supports any GL draw calls
within its preset implementations.

All presets for `libopendrop` are written natively in C++ and GLSL, and thus are
significantly more performant than equivalent presets running in either
Milkdrop2 or ProjectM.

## Building

_**Note:** `libopendrop` started as a replacement for ProjectM in my LED suit
project, and as such the build system is currently designed for
cross-compilation support on Raspberry Pi targets. The build system is a bit
messy as a result; I am working on cleaning this up._

To build `libopendrop`, first install the prerequisites:

```
sudo apt-get install golang-go libpulse-dev libgl-dev libglm-dev \
                     libncurses5-dev libncurses5 libsdl2-dev \
                     python3-pip

go get github.com/bazelbuild/bazelisk

sudo -H pip3 install absl-py

cat >>~/.bashrc << EOF
export PATH=\$PATH:\$HOME/go/bin
EOF
source ~/.bashrc

git clone https://github.com/fughilli/rpi_bazel
cd rpi_bazel
git clone https://github.com/fughilli/LedSuitDisplayDriver led_driver
git clone https://github.com/fughilli/libopendrop
git clone https://github.com/fughilli/ugfx
```

Then, build the binary target with `bazelisk`:

```
bazelisk build //libopendrop:main -c opt --copt=-I/usr/include/SDL2
```

If all goes well, there should now be a binary in the `bazel-bin` directory:

```
ls bazel-bin/libopendrop/main
```

You can run it directly, or using `bazelisk`:

```
export SOURCE=$(pactl list sources | grep Name | grep monitor | head -n 1 | awk '{ print $2 }')
bazelisk run //libopendrop:main -c opt --copt=-I/usr/include/SDL2 -- --pulseaudio_source=$SOURCE

```

### Cross-Compilation

To compile for Raspberry Pi, simply add `--config=pi` to the build command:

```
bazelisk build --config=pi -c opt //libopendrop:main
```

You will need to bootstrap the Raspberry Pi with the necessary libraries and
rebuild the sysroot used by the `rpi_bazel` workspace tooling first; see the
README for `LedSuitDisplayDriver` for more details.

## Preset Authoring

To add new presets, you can build from the example preset under
`libopendrop/preset/simple_preset`. This preset uses three GPU shaders,
`warp.fsh`, `composite.fsh`, and `passthrough.vsh`, and a single CPU shader,
`simple_preset.cc`. The preset renders a single pass of a (left, right) -> XY
waveform using `warp.fsh` targeting a back-buffer texture. `warp.fsh` is also
provided the previously-rendered output from `warp.fsh` at the last frame, which
is sampled as `last_frame`. The resulting texture is then sampled by
`composite.fsh` to draw the final result to the display.

See the shader implementation comments for more details.

### Demo Video

[![Watch the video](https://img.youtube.com/vi/-21v8h5zDC4/2.jpg)](https://www.youtube.com/watch?v=-21v8h5zDC4)
