load("//model:model_defs.bzl", "model_cc_library")
load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "space_whale_eye_warp",
    srcs = ["space_whale_eye_warp.cc"],
    hdrs = ["space_whale_eye_warp.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough_frag",
        ":passthrough_vert",
        ":warp",
        "//debug:control_injector",
        "//debug:signal_scope",
        "//preset",
        "//preset/common:outline_model",
        "//primitive:model",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util:enums",
        "//util/audio:beat_estimator",
        "//util/graphics:colors",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/logging",
        "//util/math:perspective",
        "//util/math:vector",
        "//util/signal:filter",
        "//util/signal:signals",
        "//util/signal:transition_controller",
        "//util/status:status_macros",
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
