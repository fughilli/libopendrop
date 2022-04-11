load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "gl_util",
    srcs = ["gl_util.cc"],
    hdrs = ["gl_util.h"],
    linkstatic = 1,
    deps = [
        "//third_party:gl_helper",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
    ],
)

cc_library(
    name = "colors",
    srcs = ["colors.cc"],
    hdrs = ["colors.h"],
    linkstatic = 1,
    deps = ["//third_party:glm_helper"],
)

cc_test(
    name = "colors_test",
    srcs = ["colors_test.cc"],
    deps = [
        ":colors",
        "//util/logging",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "gl_interface",
    srcs = ["gl_interface.cc"],
    hdrs = ["gl_interface.h"],
    linkstatic = 1,
    deps = [
        "//third_party:gl_helper",
        "//util/logging",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "gl_render_target",
    srcs = ["gl_render_target.cc"],
    hdrs = ["gl_render_target.h"],
    linkstatic = 1,
    deps = [
        ":gl_interface",
        ":gl_texture_manager",
        "//third_party:glm_helper",
        "//util/logging",
        "//util/status:status_macros",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "gl_texture_manager",
    srcs = ["gl_texture_manager.cc"],
    hdrs = ["gl_texture_manager.h"],
    linkstatic = 1,
    deps = [
        "//third_party:gl_helper",
        "//util/logging",
        "@com_google_absl//absl/status:statusor",
    ],
)
