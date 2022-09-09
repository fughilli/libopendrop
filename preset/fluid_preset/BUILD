load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "fluid_preset",
    srcs = ["fluid_preset.cc"],
    hdrs = ["fluid_preset.h"],
    linkstatic = 1,
    deps = [
        ":fluid",
        ":input",
        ":output",
        ":passthrough",
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
        "//util/math:vector",
        "//util/status:status_macros",
    ],
)

shader_cc_library(
    name = "passthrough",
    srcs = ["passthrough.vsh"],
)

shader_cc_library(
    name = "input",
    srcs = ["input.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "output",
    srcs = ["output.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "fluid",
    srcs = ["fluid.fsh"],
    deps = ["//preset/common:math"],
)
