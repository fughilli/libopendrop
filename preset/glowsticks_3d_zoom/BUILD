load("//libopendrop/preset:preset_defs.bzl", "shader_cc_library")

package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "glowsticks_3d_zoom",
    srcs = ["glowsticks_3d_zoom.cc"],
    hdrs = ["glowsticks_3d_zoom.h"],
    copts = [
        "-DGLM_ENABLE_EXPERIMENTAL",
    ],
    linkstatic = 1,
    deps = [
        ":composite",
        ":model",
        ":model_vertex",
        ":passthrough",
        ":ribbon",
        ":warp",
        "//libopendrop:gl_interface",
        "//libopendrop:gl_render_target",
        "//libopendrop/preset",
        "//libopendrop/primitive:polyline",
        "//libopendrop/primitive:rectangle",
        "//libopendrop/primitive:ribbon",
        "//libopendrop/util:accumulator",
        "//libopendrop/util:coefficients",
        "//libopendrop/util:colors",
        "//libopendrop/util:gl_util",
        "//libopendrop/util:logging",
        "//libopendrop/util:math",
        "//libopendrop/util:oneshot",
        "//libopendrop/util:status_macros",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:span",
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
    name = "ribbon",
    srcs = ["ribbon.fsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
)

shader_cc_library(
    name = "model",
    srcs = ["model.fsh"],
)

shader_cc_library(
    name = "model_vertex",
    srcs = ["model.vsh"],
)
