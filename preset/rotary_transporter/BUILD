load("//preset:preset_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "rotary_transporter",
    srcs = ["rotary_transporter.cc"],
    hdrs = ["rotary_transporter.h"],
    copts = [
        "-DGLM_ENABLE_EXPERIMENTAL",
    ],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough",
        ":warp",
        "//util/math:vector",
        "//util/math:perspective",
        "//:gl_interface",
        "//:gl_render_target",
        "//preset",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//util:coefficients",
        "//util:colors",
        "//util:filter",
        "//util:gl_util",
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
    name = "passthrough",
    srcs = ["passthrough.vsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
)
