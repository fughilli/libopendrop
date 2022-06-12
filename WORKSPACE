workspace(name = "libopendrop")

BAZEL_VERSION = "5.1.0"

BAZEL_VERSION_SHA = "753434f4fa730266cf5ce21d1fdd425e1e167dd9347ad3e8adc19e8c0d54edca"

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load(
    "@bazel_tools//tools/build_defs/repo:git.bzl",
    "git_repository",
    "new_git_repository",
)

git_repository(
    name = "com_google_protobuf",
    commit = "70b02861f8e8ba711efd187188dfb930db7bcaba",
    patch_args = ["-p1"],
    patches = ["@//third_party:additional_deps.patch"],
    remote = "https://github.com/protocolbuffers/protobuf",
    shallow_since = "1598416407 -0700",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

git_repository(
    name = "com_google_absl",
    commit = "60d00a5822bb98f18e40b294554f91ca14fb793a",
    remote = "https://github.com/abseil/abseil-cpp",
    shallow_since = "1602701537 -0400",
)

git_repository(
    name = "com_googletest",
    commit = "6b74da4757a549563d7c37c8fae3e704662a043b",
    remote = "https://github.com/google/googletest",
    shallow_since = "1640621124 -0800",
)

git_repository(
    name = "platforms",
    commit = "fbd0d188dac49fbcab3d2876a2113507e6fc68e9",
    remote = "https://github.com/bazelbuild/platforms",
    shallow_since = "1644333305 -0500",
)

git_repository(
    name = "com_google_re2",
    commit = "3be7d1b6b486ecd47b0daa58210051e29fb31122",
    remote = "https://github.com/google/re2",
    shallow_since = "1647366980 +0000",
)

http_archive(
    name = "rules_python",
    sha256 = "cd6730ed53a002c56ce4e2f396ba3b3be262fd7cb68339f0377a45e8227fe332",
    url = "https://github.com/bazelbuild/rules_python/releases/download/0.5.0/rules_python-0.5.0.tar.gz",
)

load("@rules_python//python:pip.bzl", "pip_parse")

pip_parse(
    name = "py_deps",
    requirements_lock = "//:requirements.txt",
)

load("@py_deps//:requirements.bzl", "install_deps")

install_deps()

git_repository(
    name = "rpi_bazel",
    commit = "1006f98cd7a1b2d1cfb050c56962981b2d5ce633",
    remote = "https://github.com/fughilli/rpi_bazel",
    shallow_since = "1604375754 -0800",
)

load("@rpi_bazel//tools/workspace:default.bzl", "add_default_repositories")

add_default_repositories()

http_archive(
    name = "hedron_compile_commands",
    sha256 = "8603191949837cd01a91a0e78c32488d781de72bcbf455c9cca79ac03160c6de",
    strip_prefix = "bazel-compile-commands-extractor-d8ff4bd0142f70e0c51b11d6297e97b81136b018",
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/d8ff4bd0142f70e0c51b11d6297e97b81136b018.tar.gz",
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()

new_local_repository(
    name = "sdl2",
    build_file_content = """
package(default_visibility = ["//visibility:public"])

cc_library(
name = "sdl2",
srcs = ["lib/libSDL2.a"],
hdrs = glob(["include/**/*.h"]),
linkopts = [
  "-framework", "CoreHaptics",
  "-framework", "ForceFeedback",
  "-framework", "GameController",
],
includes = ["include", "include/SDL2"],
)
  """,
    path = "/opt/homebrew/Cellar/sdl2/2.0.22",
)

new_local_repository(
    name = "imgui",
    build_file = "@//third_party:imgui.BUILD",
    path = "/Users/kbalke/Projects/personal/imgui",
)

# new_git_repository(
#     name = "imgui",
#     build_file = "@//third_party:imgui.BUILD",
#     commit = "4789c7e485244aa6489f89dbb03b19d4ad0ea1ec",
#     init_submodules = True,
#     remote = "https://github.com/ocornut/imgui",
#     shallow_since = "1644319784 +0100",
# )

new_git_repository(
    name = "implot",
    build_file = "@//third_party:implot.BUILD",
    commit = "b47c8bacdbc78bc521691f70666f13924bb522ab",
    init_submodules = True,
    remote = "https://github.com/epezent/implot",
    shallow_since = "1643591232 -0800",
)

git_repository(
    name = "fix_guards",
    commit = "0a1142d81740a9f596d795654a2dab602a976ffa",
    remote = "https://github.com/fughilli/fix_guards",
    shallow_since = "1648239566 -0700",
)

new_git_repository(
    name = "imgui_node_editor",
    branch = "master",
    build_file = "@//third_party:imgui_node_editor.BUILD",
    init_submodules = True,
    remote = "https://github.com/fughilli/imgui-node-editor",
)

# http_archive(
#     name = "bazelregistry_sdl2",
#     sha256 = "735b86e808d78c3a6e7db86c4532140be4ad5d7349feea2dbfef7ea1382c31eb",
#     strip_prefix = "sdl2-c3efa24f546f0d8be97aaf1609688905e585cd69",
#     urls = ["https://github.com/bazelregistry/sdl2/archive/c3efa24f546f0d8be97aaf1609688905e585cd69.zip"],
# )

new_local_repository(
    name = "glm",
    build_file_content = """
package(default_visibility = ["//visibility:public"])

cc_library(
name = "glm",
hdrs = glob(["include/**/*.h", "include/**/*.hpp", "include/**/*.inl"]),
includes = ["include"],
)
  """,
    path = "/opt/homebrew/Cellar/glm/0.9.9.8",
)

new_local_repository(
    name = "gl",
    build_file_content = """
package(default_visibility = ["//visibility:public"])

genrule(
name = "copy_lib",
srcs = ["Libraries/libGL.tbd"],
outs = ["Libraries/libGL.a"],
cmd = "cp $(SRCS) $(OUTS)",
)

cc_library(
name = "gl",
srcs = ["Libraries/libGL.a"],
hdrs = glob(["Headers/**/*.h"]),
includes = ["Headers"],
)
  """,
    path = "/Library/Developer/CommandLineTools/SDKs/MacOSX11.3.sdk/System/Library/Frameworks/OpenGL.framework/Versions/A",
)

new_local_repository(
    name = "pulseaudio",
    build_file_content = """
package(default_visibility = ["//visibility:public"])

cc_library(
name = "pulseaudio",
srcs = ["lib/libpulse.0.dylib"],
hdrs = glob(["include/**/*.h"]),
linkopts = [
  "-framework", "CoreAudio",
  "-framework", "AudioToolbox",
],
includes = ["include"],
)
  """,
    path = "/opt/homebrew/Cellar/pulseaudio/14.2",
)
