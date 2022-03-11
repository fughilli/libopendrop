load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")
load("//preset:preset_defs.bzl", "shader_cc_library")
load(
    "//toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_COPTS = "COPTS",
    CROSS_COMPILATION_DEPS = "DEPS",
    CROSS_COMPILATION_LINKOPTS = "LINKOPTS",
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
        "-Wl,-z,notext",
        "-lSDL2",
        "-lasound",
        "-lpulse",
        "-lX11",
        "-lXext",
        "-lXt",
        "-lsndio",
        "-lXcursor",
        "-lXinerama",
        "-lXrandr",
        "-lwayland-cursor",
        "-lwayland-client",
        "-lwayland-egl",
        "-lxkbcommon",
        "-lXxf86vm",
        "-lXss",
        "-lXi",
        "-lGL",
    ] + CROSS_COMPILATION_LINKOPTS,
    linkstatic = 1,
    deps = [
        ":main_lib",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "main_lib",
    srcs = ["main.cc"],
    linkstatic = 1,
    deps = [
        ":cleanup",
        ":gl_interface",
        ":open_drop_controller",
        ":open_drop_controller_interface",
        ":sdl_gl_interface",
        "//debug:signal_scope",
        "//preset:preset_list",
        "//util:logging",
        "//util:performance_timer",
        "//util:pulseaudio_interface",
        "//util:rate_limiter",
        "@com_google_absl//absl/debugging:failure_signal_handler",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
        "@imgui",
        "@imgui//:imgui_backend_opengl2",
        "@imgui//:imgui_backend_sdl",
        "@implot",
    ],
)

cc_library(
    name = "sdl_gl_interface",
    srcs = ["sdl_gl_interface.cc"],
    hdrs = ["sdl_gl_interface.h"],
    linkstatic = 1,
    deps = [":gl_interface"],
)

cc_library(
    name = "gl_interface",
    srcs = ["gl_interface.cc"],
    hdrs = ["gl_interface.h"],
    linkstatic = 1,
    deps = [
        "//util:logging",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "gl_render_target",
    srcs = ["gl_render_target.cc"],
    hdrs = ["gl_render_target.h"],
    linkstatic = 1,
    deps = [
        ":gl_interface",
        ":gl_texture_manager",
        "//util:logging",
        "//util:status_macros",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "gl_texture_manager",
    srcs = ["gl_texture_manager.cc"],
    hdrs = ["gl_texture_manager.h"],
    linkstatic = 1,
    deps = [
        "//util:logging",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "open_drop_controller_interface",
    hdrs = ["open_drop_controller_interface.h"],
    deps = [
        ":audio_processor",
        ":gl_interface",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "open_drop_controller",
    srcs = ["open_drop_controller.cc"],
    hdrs = ["open_drop_controller.h"],
    linkstatic = 1,
    deps = [
        ":blit_fsh",
        ":blit_vsh",
        ":gl_render_target",
        ":global_state",
        ":normalizer",
        ":open_drop_controller_interface",
        "//preset",
        "//preset:preset_blender",
        "//primitive:rectangle",
        "//util:gl_util",
        "//util:logging",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "cleanup",
    hdrs = ["cleanup.h"],
)

cc_library(
    name = "audio_processor",
    srcs = ["audio_processor.cc"],
    hdrs = ["audio_processor.h"],
    linkstatic = 1,
    deps = [
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "global_state",
    srcs = ["global_state.cc"],
    hdrs = ["global_state.h"],
    linkstatic = 1,
    deps = [
        "//util:accumulator",
        "//util:filter",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "normalizer",
    hdrs = ["normalizer.h"],
    deps = [
        "//debug:signal_scope",
        "//util:logging",
        "@com_google_absl//absl/types:span",
    ],
)

shader_cc_library(
    name = "blit_fsh",
    srcs = ["blit.fsh"],
)

shader_cc_library(
    name = "blit_vsh",
    srcs = ["blit.vsh"],
)
