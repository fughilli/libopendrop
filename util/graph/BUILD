load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
    CROSS_COMPILATION_LINKOPTS = "LINKOPTS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "graph_builtins",
    srcs = ["graph_builtins.cc"],
    hdrs = ["graph_builtins.h"],
    deps = [":graph"],
)

cc_library(
    name = "graph",
    hdrs = ["graph.h"],
    deps = [
        "//util/logging",
        "//third_party:gl_helper",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "graph_test",
    srcs = ["graph_test.cc"],
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
    deps = [
        "//util/math",
        "//util/graphics:colors",
        "//util/graphics:gl_util",
        "//third_party:gl_helper",
        ":graph",
        "@com_google_absl//absl/strings",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
        "//util/logging",
        ":graph_builtins",
    ] + CROSS_COMPILATION_DEPS,
)
