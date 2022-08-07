load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "logging",
    hdrs = ["logging.h"],
    deps = [
        ":logging_glm_helpers",
        ":logging_helpers",
        ":logging_macros",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
    ],
)

cc_library(
    name = "logging_macros",
    hdrs = ["logging_macros.h"],
    deps = [
      "//util/time:periodic",
    ],
)

cc_library(
    name = "logging_helpers",
    hdrs = ["logging_helpers.h"],
    deps = ["@com_google_absl//absl/types:span"],
)

cc_library(
    name = "logging_glm_helpers",
    srcs = ["logging_glm_helpers.cc"],
    hdrs = ["logging_glm_helpers.h"],
    deps = [
        "//third_party:glm_helper",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "logging_glm_helpers_test",
    srcs = ["logging_glm_helpers_test.cc"],
    deps = [
        "//third_party:glm_helper",
        ":logging_glm_helpers",
        "//util/testing:re2_matchers",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)
