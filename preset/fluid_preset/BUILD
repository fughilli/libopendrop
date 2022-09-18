load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "fluid_preset",
    srcs = ["fluid_preset.cc"],
    hdrs = ["fluid_preset.h"],
    linkstatic = 1,
    deps = [
        ":advection",
        ":blit",
        ":clear",
        ":common_vertex",
        ":curl",
        ":divergence",
        ":gradient_subtract",
        ":input",
        ":pressure",
        ":vorticity",
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
    name = "common_vertex",
    srcs = ["common_vertex.vsh"],
)

shader_cc_library(
    name = "curl",
    srcs = ["curl.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "vorticity",
    srcs = ["vorticity.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "divergence",
    srcs = ["divergence.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "clear",
    srcs = ["clear.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "pressure",
    srcs = ["pressure.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "gradient_subtract",
    srcs = ["gradient_subtract.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "advection",
    srcs = ["advection.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "blit",
    srcs = ["blit.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "input",
    srcs = ["input.fsh"],
    deps = ["//preset/common:math"],
)
