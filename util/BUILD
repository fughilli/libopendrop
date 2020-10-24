package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "logging",
    hdrs = ["logging.h"],
    linkstatic = 1,
    deps = [
        ":logging_helpers",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "logging_helpers",
    hdrs = ["logging_helpers.h"],
    deps = ["@com_google_absl//absl/types:span"],
)

cc_library(
    name = "colors",
    srcs = ["colors.cc"],
    hdrs = ["colors.h"],
    linkstatic = 1,
)

cc_library(
    name = "gl_util",
    srcs = ["gl_util.cc"],
    hdrs = ["gl_util.h"],
    linkstatic = 1,
    deps = [
        "//libopendrop:gl_interface",
        "//libopendrop:gl_render_target",
    ],
)

cc_library(
    name = "oneshot",
    hdrs = ["oneshot.h"],
)

cc_library(
    name = "coefficients",
    hdrs = ["coefficients.h"],
)

cc_library(
    name = "interpolator",
    hdrs = ["interpolator.h"],
    deps = [":logging"],
)

cc_test(
    name = "interpolator_test",
    srcs = ["interpolator_test.cc"],
    deps = [
        ":interpolator",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
        "@org_llvm_libcxx//:libcxx",
    ],
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
        "@org_llvm_libcxx//:libcxx",
    ],
)

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
        "@org_llvm_libcxx//:libcxx",
    ],
)
