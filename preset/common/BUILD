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
        ":eyeball_ball",
        ":eyeball_iris",
        ":eyeball_pupil",
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
    scale = 0.4,
    srcs = ["pill_shadow.obj"],
)

model_cc_library(
    name = "pill_center",
    scale = 0.4,
    srcs = ["pill_center.obj"],
)

model_cc_library(
    name = "pill_end_top",
    scale = 0.4,
    srcs = ["pill_end_top.obj"],
)

model_cc_library(
    name = "pill_end_bottom",
    scale = 0.4,
    srcs = ["pill_end_bottom.obj"],
)

model_cc_library(
    name = "monkey",
    normalize = True,
    srcs = ["monkey.obj"],
)

model_cc_library(
    name = "shrek",
    normalize = True,
    srcs = ["shrek.obj"],
)

model_cc_library(
    name = "alpaca_outline",
    scale = 0.450851,
    srcs = ["alpaca_outline.obj"],
)

model_cc_library(
    name = "alpaca",
    scale = 0.450851,
    srcs = ["alpaca.obj"],
)

model_cc_library(
    name = "star_outline",
    scale = 0.547779,
    srcs = ["star_outline.obj"],
)

model_cc_library(
    name = "star",
    scale = 0.547779,
    srcs = ["star.obj"],
)

model_cc_library(
    name = "cube_outline",
    scale = 0.707107,
    srcs = ["cube_outline.obj"],
)

model_cc_library(
    name = "cube",
    scale = 0.707107,
    srcs = ["cube.obj"],
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
    #scale = 1.101379,
    normalize = True,
    srcs = ["lo_x.obj"],
)
