load("//libopendrop/preset:preset_defs.bzl", "shader_cc_library")

package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "eye_roll",
    srcs = ["eye_roll.cc"],
    hdrs = ["eye_roll.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":line",
        ":ngon",
        ":passthrough",
        ":warp",
        "//libopendrop:gl_interface",
        "//libopendrop:gl_render_target",
        "//libopendrop/preset",
        "//libopendrop/primitive:ngon",
        "//libopendrop/primitive:polyline",
        "//libopendrop/primitive:rectangle",
        "//libopendrop/util:coefficients",
        "//libopendrop/util:colors",
        "//libopendrop/util:filter",
        "//libopendrop/util:gl_util",
        "//libopendrop/util:logging",
        "//libopendrop/util:math",
        "//libopendrop/util:signals",
        "//libopendrop/util:status_macros",
        "@com_google_absl//absl/strings:str_format",
    ],
)

shader_cc_library(
    name = "composite",
    srcs = ["composite.fsh"],
)

shader_cc_library(
    name = "ngon",
    srcs = ["ngon.fsh"],
)

shader_cc_library(
    name = "line",
    srcs = ["line.fsh"],
)

shader_cc_library(
    name = "passthrough",
    srcs = ["passthrough.vsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
    deps = [
        "//libopendrop/preset/common:math",
    ],
)
