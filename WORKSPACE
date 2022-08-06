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
    name = "subpar",
    commit = "15ddc75b94b19ea4f544cf5d3e640cd37a3a8b4b",
    remote = "https://github.com/fughilli/subpar",
    shallow_since = "1590903243 -0700",
)

git_repository(
    name = "com_google_protobuf",
    commit = "c9869dc7803eb0a21d7e589c40ff4f9288cd34ae",
    patch_args = ["-p1"],
    patches = ["@//third_party:additional_deps.patch"],
    remote = "https://github.com/protocolbuffers/protobuf",
    shallow_since = "1658780535 -0700",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

git_repository(
    name = "com_google_absl",
    commit = "0c92330442d6b1be934e0407115c8084250ef347",
    remote = "https://github.com/abseil/abseil-cpp",
    shallow_since = "1659719415 -0700",
)

git_repository(
    name = "com_googletest",
    commit = "5b909beeec178f338be997830b6c31a80cda7a93",
    remote = "https://github.com/google/googletest",
    shallow_since = "1659635637 -0700",
)

git_repository(
    name = "com_google_re2",
    commit = "a23f85cae66516b22fd35ba7b8f518133e4b68c4",
    remote = "https://github.com/google/re2",
    shallow_since = "1659694839 +0000",
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_python",
    sha256 = "a3a6e99f497be089f81ec082882e40246bfd435f52f4e82f37e89449b04573f6",
    strip_prefix = "rules_python-0.10.2",
    url = "https://github.com/bazelbuild/rules_python/archive/refs/tags/0.10.2.tar.gz",
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
    sha256 = "8043456c9cdb084857628e1edbb92ecd15ea1cef93b9cf08745dd4f716dcc057",
    strip_prefix = "bazel-compile-commands-extractor-e1e25d8d4827bfc7bf53d1aed35e8e28457ba96d",
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/e1e25d8d4827bfc7bf53d1aed35e8e28457ba96d.tar.gz",
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()

new_git_repository(
    name = "imgui",
    build_file = "@//third_party:imgui.BUILD",
    commit = "aa8680009248061c83f3d6722ec53c1a320d872b",
    init_submodules = True,
    remote = "https://github.com/ocornut/imgui",
    shallow_since = "1644319784 +0100",
)

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

new_local_repository(
    name = "sdl2",
    build_file_content = """
package(default_visibility = ["//visibility:public"])
cc_library(
name = "sdl2",
srcs = ["lib/x86_64-linux-gnu/libSDL2.a"],
hdrs = glob(["include/SDL2/*.h"]),
includes = ["include", "include/SDL2"],
)
  """,
    path = "/usr",
)

new_local_repository(
    name = "gl",
    build_file_content = """
package(default_visibility = ["//visibility:public"])
cc_library(
name = "gl",
srcs = ["lib/x86_64-linux-gnu/libGL.so"],
hdrs = glob(["include/GL/**/*.h"]),
includes = ["include", "include/GL"],
)
  """,
    path = "/usr",
)
