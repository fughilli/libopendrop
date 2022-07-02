load("//shader:shader_defs.bzl", "shader_cc_library")

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "graph_preset",
    srcs = ["graph_preset.cc"],
    hdrs = ["graph_preset.h"],
    linkstatic = 1,
    deps = [
        ":model_frag",
        ":passthrough_vert",
        ":tile_frag",
        ":zoom_frag",
        "//preset",
        "//primitive:polyline",
        "//primitive:rectangle",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util/graph",
        "//util/graph:rendering",
        "//util/graph/types",
        "//util/graph/types:color",
        "//util/graph/types:monotonic",
        "//util/graph/types:texture",
        "//util/graph/types:unitary",
        "//util/graphics:colors",
        "//util/graphics:gl_interface",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/logging",
        "//util/math",
        "//util/math:coefficients",
        "//util/math:perspective",
        "//util/math:vector",
        "//util/status:status_macros",
        "@imgui",
        "@imgui_node_editor",
    ],
)

shader_cc_library(
    name = "passthrough_vert",
    srcs = ["passthrough_vert.vsh"],
)

shader_cc_library(
    name = "model_frag",
    srcs = ["model_frag.fsh"],
)

shader_cc_library(
    name = "tile_frag",
    srcs = ["tile_frag.fsh"],
    deps = ["//preset/common:math"],
)

shader_cc_library(
    name = "zoom_frag",
    srcs = ["zoom_frag.fsh"],
    deps = ["//preset/common:math"],
)
