load("//libopendrop/preset:preset_defs.bzl", "shader_cc_library")

package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "glowsticks_3d",
    srcs = ["glowsticks_3d.cc"],
    hdrs = ["glowsticks_3d.h"],
    linkstatic = 1,
    deps = [
        ":composite",
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
