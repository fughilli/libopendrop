load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "coefficients",
    hdrs = ["coefficients.h"],
    deps = ["//util/logging"],
)

cc_library(
    name = "constants",
    srcs = ["constants.cc"],
    hdrs = ["constants.h"],
    deps = ["//third_party:glm_helper"],
)

cc_library(
    name = "perspective",
    srcs = ["perspective.cc"],
    hdrs = ["perspective.h"],
    deps = ["//third_party:glm_helper"],
)

cc_library(
    name = "vector",
    srcs = ["vector.cc"],
    hdrs = ["vector.h"],
    deps = [
        ":constants",
        ":math",
        ":perspective",
        "//third_party:glm_helper",
        "//util/logging",
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
        "//third_party:glm_helper",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "math",
    srcs = ["math.cc"],
    hdrs = ["math.h"],
    deps = ["//util/logging"],
)

cc_test(
    name = "math_test",
    srcs = ["math_test.cc"],
    deps = [
        ":math",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)
