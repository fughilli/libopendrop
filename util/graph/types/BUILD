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
    name = "opaque_storable",
    hdrs = ["opaque_storable.h"],
    deps = ["//util/logging"],
)

cc_test(
    name = "opaque_storable_test",
    srcs = ["opaque_storable_test.cc"],
    deps = [
        ":opaque_storable",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "monotonic",
    hdrs = ["monotonic.h"],
    deps = [
        ":opaque_storable",
        ":types",
    ],
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
        ":opaque_storable",
        ":types",
        "//util/logging",
    ],
)

cc_library(
    name = "texture",
    srcs = ["texture.cc"],
    hdrs = ["texture.h"],
    deps = [
        ":opaque_storable",
        ":types",
        "//primitive:rectangle",
        "//shader:blit_fsh",
        "//shader:blit_vsh",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_texture_manager",
        "//util/graphics:gl_util",
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
