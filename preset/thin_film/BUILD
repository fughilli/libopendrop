load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "thin_film",
    srcs = ["thin_film.cc"],
    hdrs = ["thin_film.h"],
    linkstatic = 1,
    deps = [
        ":bubble",
        ":passthrough",
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
        "//util/math:vector",
        "//util/signal:filter",
        "//util/status:status_macros",
    ],
)

shader_cc_library(
    name = "bubble",
    srcs = ["bubble.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "passthrough",
    srcs = ["passthrough.vsh"],
)
