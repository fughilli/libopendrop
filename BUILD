load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")

VIDEOCORE_COPTS = [
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include",
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include/interface/vcos/pthreads",
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include/interface/vmcs_host/linux",
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
        ":gl_interface",
        ":open_drop_controller",
        ":open_drop_controller_interface",
        ":sdl_gl_interface",
        "//led_driver:performance_timer",
        "//led_driver:pulseaudio_interface",
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
)

cc_library(
    name = "open_drop_controller_interface",
    hdrs = ["open_drop_controller_interface.h"],
    deps = [":gl_interface"],
)

cc_library(
    name = "open_drop_controller",
    srcs = ["open_drop_controller.cc"],
    hdrs = ["open_drop_controller.h"],
    linkstatic = 1,
    deps = [
        ":open_drop_controller_interface",
        "@com_google_absl//absl/types:span",
    ],
)
