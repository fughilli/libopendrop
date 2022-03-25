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
    patches = ["@//:additional_deps.patch"],
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
