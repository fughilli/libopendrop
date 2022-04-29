load("//shader:shader_defs.bzl", "shader_cc_library")
load("//model:model_defs.bzl", "model_cc_library")

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
        ":alpaca",
        ":alpaca_outline",
        ":cube",
        ":cube_outline",
        ":eyeball_ball",
        ":eyeball_iris",
        ":eyeball_pupil",
        ":head_inner",
        ":head_outer",
        ":jaw_inner",
        ":jaw_outer",
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
        "//debug:control_injector",
        "//primitive:model",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/math:vector",
        "//util/status:status_macros",
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
    scale = 0.4,
)

model_cc_library(
    name = "pill_center",
    srcs = ["pill_center.obj"],
    scale = 0.4,
)

model_cc_library(
    name = "pill_end_top",
    srcs = ["pill_end_top.obj"],
    scale = 0.4,
)

model_cc_library(
    name = "pill_end_bottom",
    srcs = ["pill_end_bottom.obj"],
    scale = 0.4,
)

model_cc_library(
    name = "monkey",
    srcs = ["monkey.obj"],
    normalize = True,
)

model_cc_library(
    name = "shrek",
    srcs = ["shrek.obj"],
    normalize = True,
)

model_cc_library(
    name = "alpaca_outline",
    srcs = ["alpaca_outline.obj"],
    scale = 0.450851,
)

model_cc_library(
    name = "alpaca",
    srcs = ["alpaca.obj"],
    scale = 0.450851,
)

model_cc_library(
    name = "star_outline",
    srcs = ["star_outline.obj"],
    scale = 0.547779,
)

model_cc_library(
    name = "star",
    srcs = ["star.obj"],
    scale = 0.547779,
)

model_cc_library(
    name = "cube_outline",
    srcs = ["cube_outline.obj"],
    scale = 0.707107,
)

model_cc_library(
    name = "cube",
    srcs = ["cube.obj"],
    scale = 0.707107,
)

model_cc_library(
    name = "eyeball_pupil",
    srcs = ["eyeball_pupil.obj"],
    scale = 0.707107,
)

model_cc_library(
    name = "eyeball_iris",
    srcs = ["eyeball_iris.obj"],
    scale = 0.707107,
)

model_cc_library(
    name = "eyeball_ball",
    srcs = ["eyeball_ball.obj"],
    scale = 0.707107,
)

model_cc_library(
    name = "lo_x",
    srcs = ["lo_x.obj"],
    normalize = True,
)

model_cc_library(
    name = "head_outer",
    srcs = ["head_outer.obj"],
    scale = 0.707107,
)

model_cc_library(
    name = "head_inner",
    srcs = ["head_inner.obj"],
    scale = 0.707107,
)

model_cc_library(
    name = "jaw_outer",
    srcs = ["jaw_outer.obj"],
    scale = 0.707107,
)

model_cc_library(
    name = "jaw_inner",
    srcs = ["jaw_inner.obj"],
    scale = 0.707107,
)
