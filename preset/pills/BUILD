load("//model:model_defs.bzl", "model_cc_library")
load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "pills",
    srcs = ["pills.cc"],
    hdrs = ["pills.h"],
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
        "//util/graphics:colors",
        "//util:enums",
        "//third_party:gl_helper",
        "//util/graphics:gl_util",
        "//third_party:glm_helper",
        "//util/logging",
        "//util/signal:signals",
        "//util/status:status_macros",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
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
