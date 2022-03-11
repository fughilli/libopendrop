load("//preset:preset_defs.bzl", "model_cc_library", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

shader_cc_library(
    name = "math",
    hdrs = ["math.shh"],
)

cc_library(
    name = "outline_model",
    srcs = ["outline_model.cc"],
    hdrs = ["outline_model.h"],
    deps = [
      "//debug:control_injector",
        ":alpaca",
        ":alpaca_outline",
        ":cube",
        ":cube_outline",
        ":lo_x",
        ":model",
        ":monkey",
        ":passthrough_vert",
        ":pill_center",
        ":pill_end_bottom",
        ":pill_end_top",
        ":pill_shadow",
        ":shrek",
        ":star",
        ":star_outline",
        "//:gl_interface",
        "//:gl_render_target",
        "//primitive:model",
        "//util:gl_util",
        "//util:status_macros",
    ],
)

shader_cc_library(
    name = "passthrough_vert",
    srcs = ["passthrough_vert.vsh"],
)

shader_cc_library(
    name = "model",
    srcs = ["model.fsh"],
    deps = [":math"],
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
    name = "pill_end_top",
    srcs = ["pill_end_top.obj"],
)

model_cc_library(
    name = "pill_end_bottom",
    srcs = ["pill_end_bottom.obj"],
)

model_cc_library(
    name = "monkey",
    srcs = ["monkey.obj"],
)

model_cc_library(
    name = "shrek",
    srcs = ["shrek.obj"],
)

model_cc_library(
    name = "alpaca_outline",
    srcs = ["alpaca_outline.obj"],
)

model_cc_library(
    name = "alpaca",
    srcs = ["alpaca.obj"],
)

model_cc_library(
    name = "star_outline",
    srcs = ["star_outline.obj"],
)

model_cc_library(
    name = "star",
    srcs = ["star.obj"],
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
    name = "lo_x",
    srcs = ["lo_x.obj"],
)
