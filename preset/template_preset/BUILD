load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "template_preset",
    srcs = ["template_preset.cc"],
    hdrs = ["template_preset.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough",
        ":warp",
        "//preset",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//util/graphics:colors",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/logging",
        "//util/status:status_macros",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
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
