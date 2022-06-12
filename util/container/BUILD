load(
    "//bazel/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
    CROSS_COMPILATION_LINKOPTS = "LINKOPTS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "algorithms",
    hdrs = ["algorithms.h"],
    deps = [
        "//util/logging",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "algorithms_test",
    srcs = ["algorithms_test.cc"],
    deps = [
        ":algorithms",
        "@com_googletest//:gtest",
        "//util/testing:test_main",
    ] + CROSS_COMPILATION_DEPS,
)
