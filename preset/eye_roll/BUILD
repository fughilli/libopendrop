load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "eye_roll",
    srcs = ["eye_roll.cc"],
    hdrs = ["eye_roll.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":line",
        ":ngon",
        ":passthrough",
        ":warp",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//preset",
        "//primitive:ngon",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//util/math:coefficients",
        "//util/graphics:colors",
        "//util/signal:filter",
        "//third_party:gl_helper",
        "//util/graphics:gl_util",
        "//third_party:glm_helper",
        "//util/logging",
        "//util/math",
        "//util/signal:signals",
        "//util/status:status_macros",
        "@com_google_absl//absl/strings:str_format",
    ],
)

shader_cc_library(
    name = "composite",
    srcs = ["composite.fsh"],
)

shader_cc_library(
    name = "ngon",
    srcs = ["ngon.fsh"],
)

shader_cc_library(
    name = "line",
    srcs = ["line.fsh"],
)

shader_cc_library(
    name = "passthrough",
    srcs = ["passthrough.vsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
    deps = [
        "//preset/common:math",
    ],
)
