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
    deps = [
        ":graph",
        "//util/graph/types:texture",
    ],
)

cc_library(
    name = "graph",
    hdrs = ["graph.h"],
    deps = [
        "//third_party:gl_helper",
        "//util/graph/types",
        "//util/logging",
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
        "//util/testing:graph_matchers",
        "//util/graph/types",
        "//util/graph/types:monotonic",
        "//util/graph/types:unitary",
        "//util/graph/types:texture",
    ] + CROSS_COMPILATION_DEPS,
)
