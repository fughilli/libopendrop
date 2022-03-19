load("//preset:preset_defs.bzl", "model_cc_library", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "cube_wreath",
    srcs = ["cube_wreath.cc"],
    hdrs = ["cube_wreath.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough_frag",
        ":passthrough_vert",
        ":warp",
        "//:gl_interface",
        "//:gl_render_target",
        "//debug:control_injector",
        "//preset",
        "//preset/common:outline_model",
        "//primitive:model",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//util:colors",
        "//util:enums",
        "//util:gl_helper",
        "//util:gl_util",
        "//util:glm_helper",
        "//util:logging",
        "//util:status_macros",
        "//util/math:perspective",
        "//util/math:vector",
    ],
)

shader_cc_library(
    name = "composite",
    srcs = ["composite.fsh"],
)

shader_cc_library(
    name = "passthrough_vert",
    srcs = ["passthrough_vert.vsh"],
)

shader_cc_library(
    name = "passthrough_frag",
    srcs = ["passthrough_frag.fsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
    deps = ["//preset/common:math"],
)
