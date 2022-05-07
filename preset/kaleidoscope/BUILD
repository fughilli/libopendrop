load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "kaleidoscope",
    srcs = ["kaleidoscope.cc"],
    hdrs = ["kaleidoscope.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough",
        ":warp",
        ":waveform",
        "//debug:control_injector",
        "//preset",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util/graphics:colors",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/logging",
        "//util/math",
        "//util/status:status_macros",
        "@com_google_absl//absl/status:statusor",
    ],
)

shader_cc_library(
    name = "composite",
    srcs = ["composite.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "passthrough",
    srcs = ["passthrough.vsh"],
)

shader_cc_library(
    name = "waveform",
    srcs = ["waveform.fsh"],
)

shader_cc_library(
    name = "warp",
    srcs = ["warp.fsh"],
    deps = ["//preset/common:math"],
)
