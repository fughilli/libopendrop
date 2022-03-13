load(
    "//toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "constants",
    srcs = ["constants.cc"],
    hdrs = ["constants.h"],
)

cc_library(
    name = "perspective",
    srcs = ["perspective.cc"],
    hdrs = ["perspective.h"],
)

cc_library(
    name = "vector",
    srcs = ["vector.cc"],
    hdrs = ["vector.h"],
    deps = [
        ":constants",
        "//util:logging",
        "//util:math",
        "//util/math:perspective",
    ],
)

cc_test(
    name = "vector_test",
    srcs = ["vector_test.cc"],
    deps = [
        ":vector",
        "//util/testing:glm_matchers",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)
