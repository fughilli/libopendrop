load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "graph",
    hdrs = ["graph.h"],
)

cc_test(
    name = "graph_test",
    srcs = ["graph_test.cc"],
    deps = [
        "//util/math",
        ":graph",
        "@com_google_absl//absl/strings",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
        "//util/logging",
    ] + CROSS_COMPILATION_DEPS,
)
