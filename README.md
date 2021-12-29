# libopendrop

![Build and Test Host Workflow](https://github.com/fughilli/libopendrop/actions/workflows/build_and_test.yaml/badge.svg)

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

`libopendrop` builds using Bazel. A nice way to manage Bazel installs is to use
bazelisk. Install bazelisk:

```
sudo apt-get install golang-go

go get github.com/bazelbuild/bazelisk

# Put the golang bin directory onto your PATH. The following works for bash:
cat >>~/.bashrc << EOF
export PATH=\$PATH:\$HOME/go/bin
EOF
source ~/.bashrc
```

Next, install the prerequisites for building libopendrop:

```
sudo apt-get install libpulse-dev libgl-dev libglm-dev libncurses5-dev \
                     libncurses5 libsdl2-dev python3-pip

sudo -H pip3 install absl-py
```

Use the runner program to build with bazelisk and run against the system audio
source:

```
./run_libopendrop.sh -s system
```

The `-s` flag selects which pulseaudio source to use for input to the
visualizer. Available options can be listed by passing `-s ?`:

```
./run_libopendrop.sh -s ?
```

### Demo Video

[![Watch the video](https://img.youtube.com/vi/-21v8h5zDC4/2.jpg)](https://www.youtube.com/watch?v=-21v8h5zDC4)
