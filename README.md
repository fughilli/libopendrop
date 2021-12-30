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

Next, install the prerequisites for building `libopendrop`:

```
sudo apt-get install libpulse-dev libgl-dev libglm-dev libncurses5-dev \
                     libncurses5 libsdl2-dev python3-pip
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

Consult the usage for more information:

```
./run_libopendrop.sh -h
```

### Toolchains and Cross-Compilation

`libopendrop` is configured to build for host platforms using either GNU `gcc`
or LLVM `clang`. Cross-compilation for `armeabihf` targeting Raspberry Pi is
also supported using LLVM `clang`. To select an alternative compiler, provide
the `-c` argument to `run_libopendrop.sh`:

| Argument           | Compiler     | Cross Compilation Target   |
| ------------------ | ------------ | -------------------------- |
| `-c gcc` (default) | GNU `gcc`    | None (host)                |
| `-c clang`         | LLVM `clang` | None (host)                |
| `-c pi`            | LLVM `clang` | Raspberry Pi (`armeabihf`) |

Cross-compiled binaries cannot be run on the host when the architectures are
incompatible. To build only and not run the binary afterwards, pass `-z`:

```
./run_libopendrop.sh -c pi -z
```

Cross-compilation support for Raspberry Pi is provided via a
[fork of `rpi_bazel`](https://github.com/fughilli/rpi_bazel) with a
specially-configured sysroot image captured from a Raspberry Pi Raspbian system
after installing various packages needed for other projects. This sysroot should
work for building binaries that run on Raspbian (or Raspberry Pi OS). Some
additional packages may need to be installed to match the configuration in the
sysroot.

See the installation procedure in the
[setup_pi.sh](https://github.com/fughilli/LedSuitDisplayDriver/blob/f822bf56303b67786c26f9b5d371c053456c1a00/remote_scripts/setup_pi.sh#L12)
script used to configure the Raspbian system described above for more details.

## Testing

Unit tests are implemented for the libraries in `libopendrop`. Run all tests
using:

```
./run_libopendrop.sh -t
```

The compiler for the tests can be changed with `-c` as above.

## Preset Authoring

---

## **NOTE:** The preset API is in flux and is likely to change significantly to achieve various features.

---

New presets can be forked from the existing set using
`preset/fork_template_preset.sh`:

```
cd preset
./fork_template_preset.sh [-t some_existing_preset] new_preset
```

This will generate a new directory with the name `new_preset` forking
`some_existing_preset` (or `template_preset` by default, if not specified with
`-t`). To add this new preset to `libopendrop`, add it to the list in
`preset/BUILD`:

```
PRESET_DEPS_LIST = [
  "//preset/alien_rorschach",
  "//preset/cube_boom",
  ...
  "//preset/new_preset",
]
```

Then, add a call to include the header in
[`preset/preset_list.h`](https://github.com/fughilli/libopendrop/blob/master/preset/preset_list.h):

```
// Preset includes
#include "preset/alien_rorschach/alien_rorschach.h"
#include "preset/cube_boom/cube_boom.h"
  ...
#include "preset/new_preset/new_preset.h"
```

And finally, add the class name to the list in the definition of
[`opendrop::GetRandomPresetFromList()`](https://github.com/fughilli/libopendrop/blob/49da612aaac05effe8e05ae9db5a1e2116e4bbf9/preset/preset_list.h#L94-L104):

```
  ...
return GetRandomPreset<opendrop::AlienRorschach,
                       opendrop::CubeBoom,
                         ...
                       opendrop::NewPreset>(std::forward<Args>(args)...);
  ...
```

## Demo Video

The following is a demo of `preset/cube_wreath` with input audio from Vortex by
Rezz and PEEKABOO:

[![Watch the video](https://img.youtube.com/vi/9EKmqaFf8c4/0.jpg)](https://www.youtube.com/watch?v=9EKmqaFf8c4)
