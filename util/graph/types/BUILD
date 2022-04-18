load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
    CROSS_COMPILATION_LINKOPTS = "LINKOPTS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "types",
    srcs = ["types.cc"],
    hdrs = ["types.h"],
)

cc_library(
    name = "monotonic",
    hdrs = ["monotonic.h"],
    deps = [":types"],
)

cc_library(
    name = "color",
    hdrs = ["color.h"],
    deps = [
        ":types",
        "//third_party:glm_helper",
    ],
)

cc_library(
    name = "unitary",
    hdrs = ["unitary.h"],
    deps = [
        ":types",
        "//util/logging",
    ],
)

cc_library(
    name = "texture",
    srcs = ["texture.cc"],
    hdrs = ["texture.h"],
    deps = [
        ":types",
        "//third_party:glm_helper",
        "//util/logging",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "types_test",
    srcs = ["types_test.cc"],
    deps = [
        ":types",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
        ":monotonic",
        ":unitary",
    ] + CROSS_COMPILATION_DEPS,
)
