load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "simple_preset",
    srcs = ["simple_preset.cc"],
    hdrs = ["simple_preset.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough",
        ":warp",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//preset",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//util/graphics:colors",
        "//third_party:gl_helper",
        "//util/graphics:gl_util",
        "//third_party:glm_helper",
        "//util/logging",
        "//util/math",
        "//util/status:status_macros",
        "@com_google_absl//absl/status:statusor",
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
