load("//preset:preset_defs.bzl", "model_cc_library", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "cube_wreath",
    srcs = ["cube_wreath.cc"],
    hdrs = ["cube_wreath.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":cube",
        ":cube_outline",
        ":model",
        ":monkey",
        ":passthrough_frag",
        ":passthrough_vert",
        ":shrek",
        ":warp",
        "//:gl_interface",
        "//:gl_render_target",
        "//preset",
        "//primitive:model",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//util:colors",
        "//util:gl_util",
        "//util:logging",
        "//util:status_macros",
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
    name = "model",
    srcs = ["model.fsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
)

model_cc_library(
    name = "cube_outline",
    srcs = ["cube_outline.obj"],
)

model_cc_library(
    name = "cube",
    srcs = ["cube.obj"],
)

model_cc_library(
    name = "monkey",
    srcs = ["monkey.obj"],
)

model_cc_library(
    name = "shrek",
    srcs = ["shrek.obj"],
)
