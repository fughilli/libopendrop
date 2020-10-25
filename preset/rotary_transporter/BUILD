load("//libopendrop/preset:preset_defs.bzl", "shader_cc_library")

package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "rotary_transporter",
    srcs = ["rotary_transporter.cc"],
    hdrs = ["rotary_transporter.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough",
        ":warp",
        "//libopendrop:gl_interface",
        "//libopendrop:gl_render_target",
        "//libopendrop/preset",
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
    name = "warp",
    srcs = ["warp.fsh"],
)
