load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")
load("//shader:shader_defs.bzl", "shader_cc_library")
load(
    "//bazel/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_COPTS = "COPTS",
    CROSS_COMPILATION_DEPS = "DEPS",
    CROSS_COMPILATION_LINKOPTS = "LINKOPTS",
)

filegroup(
  name = "debug_files",
  srcs = [".lldbinit"],
)

package(default_visibility = ["//:__subpackages__"])

config_setting(
    name = "pi_build",
    values = {
        "cpu": "armeabihf",
        "compiler": "clang",
    },
)

config_setting(
    name = "clang_build",
    values = {"compiler": "clang"},
)

cc_binary(
    name = "main",
    copts = CROSS_COMPILATION_COPTS,
    linkopts = [
    ] + CROSS_COMPILATION_LINKOPTS,
    linkstatic = 1,
    data = [
      ":debug_files",
    ],
    deps = [
        ":main_lib",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "main_lib",
    srcs = ["main.cc"],
    linkstatic = 1,
    deps = [
        "//application:open_drop_controller",
        "//application:open_drop_controller_interface",
        "//preset:preset_list",
        "//util:cleanup",
        "//util/audio:pulseaudio_interface",
        "//util/graphics:gl_interface",
        "//util/graphics/sdl:sdl_gl_interface",
        "//util/logging",
        "//util/time:performance_timer",
        "//util/time:rate_limiter",
        "@com_google_absl//absl/debugging:failure_signal_handler",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
        "@imgui",
        "@imgui//:imgui_backend_opengl2",
        "@imgui//:imgui_backend_sdl",
        "@pulseaudio",
        "@sdl2",
    ],
)
