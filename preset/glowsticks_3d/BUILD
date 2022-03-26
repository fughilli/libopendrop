load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

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
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//preset",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//primitive:ribbon",
        "//util/signal:accumulator",
        "//util/math:coefficients",
        "//util/graphics:colors",
        "//third_party:gl_helper",
        "//util/graphics:gl_util",
        "//third_party:glm_helper",
        "//util/logging",
        "//util/math",
        "//util/time:oneshot",
        "//util/status:status_macros",
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
