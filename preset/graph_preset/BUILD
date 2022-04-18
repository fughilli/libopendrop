load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "graph_preset",
    srcs = ["graph_preset.cc"],
    hdrs = ["graph_preset.h"],
    linkstatic = 1,
    deps = [
        ":composite",
        ":passthrough",
        ":warp",
        "//preset",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util/graph",
        "//util/graph/types",
        "//util/graph/types:color",
        "//util/graph/types:monotonic",
        "//util/graph/types:unitary",
        "//util/graphics:colors",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/logging",
        "//util/status:status_macros",
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
