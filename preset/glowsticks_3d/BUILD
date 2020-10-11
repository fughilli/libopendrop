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
        ":warp",
        "//libopendrop:gl_interface",
        "//libopendrop:gl_render_target",
        "//libopendrop/preset",
        "//libopendrop/primitive:rectangle",
        "//libopendrop/primitive:ribbon",
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
