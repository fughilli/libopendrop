load("//preset:preset_defs.bzl", "model_cc_library", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "pills",
    srcs = ["pills.cc"],
    hdrs = ["pills.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":model",
        ":monkey",
        ":passthrough_frag",
        ":passthrough_vert",
        ":pill_center",
        ":pill_ends",
        ":pill_shadow",
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
    name = "model",
    srcs = ["model.fsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
    deps = ["//preset/common:math"],
)

model_cc_library(
    name = "pill_shadow",
    srcs = ["pill_shadow.obj"],
)

model_cc_library(
    name = "pill_center",
    srcs = ["pill_center.obj"],
)

model_cc_library(
    name = "pill_ends",
    srcs = ["pill_ends.obj"],
)

model_cc_library(
    name = "monkey",
    srcs = ["monkey.obj"],
)

model_cc_library(
    name = "shrek",
    srcs = ["shrek.obj"],
)
