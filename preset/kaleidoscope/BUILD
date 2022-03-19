load("//preset:preset_defs.bzl", "shader_cc_library")

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
        "//:gl_interface",
        "//:gl_render_target",
        "//preset",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//util:colors",
        "//util:gl_helper",
        "//util:gl_util",
        "//util:glm_helper",
        "//util:logging",
        "//util:math",
        "//util:status_macros",
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
