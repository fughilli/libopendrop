load("//preset:preset_defs.bzl", "shader_cc_library")

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
        "//:gl_interface",
        "//:gl_render_target",
        "//preset",
        "//primitive:ngon",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//util:coefficients",
        "//util:colors",
        "//util:filter",
        "//util:gl_helper",
        "//util:gl_util",
        "//util:glm_helper",
        "//util:logging",
        "//util:math",
        "//util:status_macros",
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
