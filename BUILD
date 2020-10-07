load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")
load("//libopendrop/preset:preset_defs.bzl", "shader_cc_library")

package(default_visibility = ["//libopendrop:__subpackages__"])

VIDEOCORE_COPTS = [
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include",
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include/interface/vcos/pthreads",
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include/interface/vmcs_host/linux",
    "-L",
    "external/raspberry_pi/sysroot/opt/vc/lib",
]

SYSROOT_COPTS = [
    "-isystem",
    "external/raspberry_pi/sysroot/usr/include",
]

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    copts = SYSROOT_COPTS,
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
    ],
    linkstatic = 1,
    deps = [
        ":cleanup",
        ":gl_interface",
        ":open_drop_controller",
        ":open_drop_controller_interface",
        ":sdl_gl_interface",
        "//led_driver:performance_timer",
        "//led_driver:pulseaudio_interface",
        "//libopendrop/preset/alien_rorschach",
        "//libopendrop/preset/kaleidoscope",
        "//libopendrop/preset/simple_preset",
        "//libopendrop/util:logging",
        "@com_google_absl//absl/debugging:failure_signal_handler",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/types:span",
        "@org_llvm_libcxx//:libcxx",
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
    deps = ["//libopendrop/util:logging"],
)

cc_library(
    name = "gl_render_target",
    srcs = ["gl_render_target.cc"],
    hdrs = ["gl_render_target.h"],
    linkstatic = 1,
    deps = [
        ":gl_interface",
        ":gl_texture_manager",
        "//libopendrop/util:logging",
    ],
)

cc_library(
    name = "gl_texture_manager",
    srcs = ["gl_texture_manager.cc"],
    hdrs = ["gl_texture_manager.h"],
    linkstatic = 1,
    deps = ["//libopendrop/util:logging"],
)

cc_library(
    name = "open_drop_controller_interface",
    hdrs = ["open_drop_controller_interface.h"],
    deps = [
        ":audio_processor",
        ":gl_interface",
        "//libopendrop/preset",
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
        "//libopendrop/preset",
        "//libopendrop/primitive:rectangle",
        "//libopendrop/util:gl_util",
        "//libopendrop/util:logging",
        "//libopendrop/preset:preset_blender",
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
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "normalizer",
    hdrs = ["normalizer.h"],
    deps = [
        "//libopendrop/util:logging",
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
