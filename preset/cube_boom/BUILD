load("//libopendrop/preset:preset_defs.bzl", "model_cc_library", "shader_cc_library")

package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "cube_boom",
    srcs = ["cube_boom.cc"],
    hdrs = ["cube_boom.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":cube",
        ":model",
        ":monkey",
        ":shrek",
        ":passthrough",
        ":warp",
        "//libopendrop:gl_interface",
        "//libopendrop:gl_render_target",
        "//libopendrop/preset",
        "//libopendrop/primitive:model",
        "//libopendrop/primitive:polyline",
        "//libopendrop/primitive:rectangle",
        "//libopendrop/util:colors",
        "//libopendrop/util:gl_util",
        "//libopendrop/util:logging",
        "//libopendrop/util:status_macros",
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
    name = "model",
    srcs = ["model.fsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
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
