load("//model:model_defs.bzl", "model_cc_library")
load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

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
        ":passthrough",
        ":shrek",
        ":warp",
        "//preset",
        "//primitive:model",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util/graphics:colors",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/logging",
        "//util/status:status_macros",
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
