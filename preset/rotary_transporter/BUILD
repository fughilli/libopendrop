load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "rotary_transporter",
    srcs = ["rotary_transporter.cc"],
    hdrs = ["rotary_transporter.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough",
        ":warp",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//preset",
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
        "//util/status:status_macros",
        "//util/math:perspective",
        "//util/math:vector",
    ],
)

shader_cc_library(
    name = "composite",
    srcs = ["composite.fsh"],
)

shader_cc_library(
    name = "passthrough",
    srcs = ["passthrough.vsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
)
