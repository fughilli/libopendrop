load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "shape_bounce",
    srcs = ["shape_bounce.cc"],
    hdrs = ["shape_bounce.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":ngon",
        ":passthrough",
        ":warp",
        "//preset",
        "//primitive:ngon",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util/graphics:colors",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/logging",
        "//util/math",
        "//util/math:coefficients",
        "//util/signal:filter",
        "//util/status:status_macros",
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
    name = "passthrough",
    srcs = ["passthrough.vsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
)
