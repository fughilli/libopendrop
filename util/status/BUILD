load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "status_macros",
    hdrs = ["status_macros.h"],
    deps = [
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_test(
    name = "status_macros_test",
    srcs = ["status_macros_test.cc"],
    deps = [
        ":status_macros",
        "@com_google_absl//absl/status:statusor",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)
