load("//libopendrop/preset:preset_defs.bzl", "shader_cc_library")

package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "alien_rorschach",
    srcs = ["alien_rorschach.cc"],
    hdrs = ["alien_rorschach.h"],
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
