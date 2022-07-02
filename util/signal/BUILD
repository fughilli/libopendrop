load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "filter",
    srcs = ["filter.cc"],
    hdrs = ["filter.h"],
    deps = [
        "//util/logging",
        "//util/math",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "filter_test",
    srcs = ["filter_test.cc"],
    deps = [
        ":filter",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "signals",
    srcs = ["signals.cc"],
    hdrs = ["signals.h"],
    deps = [
        "//util/math",
        "//util/math:coefficients",
        "//util/time:oneshot",
    ],
)

cc_library(
    name = "interpolator",
    hdrs = ["interpolator.h"],
    deps = ["//util/logging"],
)

cc_test(
    name = "interpolator_test",
    srcs = ["interpolator_test.cc"],
    deps = [
        ":interpolator",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "accumulator",
    hdrs = ["accumulator.h"],
    deps = [":interpolator"],
)

cc_test(
    name = "accumulator_test",
    srcs = ["accumulator_test.cc"],
    deps = [
        ":accumulator",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "decay_towards",
    hdrs = ["decay_towards.h"],
)

cc_library(
    name = "transition_controller",
    hdrs = ["transition_controller.h"],
    deps = [":decay_towards"],
)

cc_library(
    name = "unitizer",
    hdrs = ["unitizer.h"],
)
