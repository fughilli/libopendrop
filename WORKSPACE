workspace(name = "libopendrop")

BAZEL_VERSION = "3.1.0"

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

# Hedron's Compile Commands Extractor for Bazel
# https://github.com/hedronvision/bazel-compile-commands-extractor
http_archive(
    name = "hedron_compile_commands",
    strip_prefix = "bazel-compile-commands-extractor-084957eaa1bf6e2bd031f50b1f5d04c89273103a",

    # Replace the commit hash in both places (below) with the latest, rather than using the stale one here.
    # Even better, set up Renovate and let it do the work for you (see "Suggestion: Updates" in the README).
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/084957eaa1bf6e2bd031f50b1f5d04c89273103a.tar.gz",
    # When you first run this tool, it'll recommend a sha256 hash to put here with a message like: "DEBUG: Rule 'hedron_compile_commands' indicated that a canonical reproducible form can be obtained by modifying arguments sha256 = ..."
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()

new_git_repository(
    name = "imgui",
    branch = "master",
    build_file = "@//third_party/imgui:BUILD",
    init_submodules = True,
    remote = "https://github.com/ocornut/imgui",
)
