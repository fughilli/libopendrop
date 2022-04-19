package(default_visibility = ["//visibility:public"])

cc_library(
    name = "imgui_node_editor",
    srcs = [
        "crude_json.cpp",
        "imgui_canvas.cpp",
        "imgui_node_editor.cpp",
        "imgui_node_editor_api.cpp",
    ],
    hdrs = [
        "crude_json.h",
        "imgui_bezier_math.h",
        "imgui_bezier_math.inl",
        "imgui_canvas.h",
        "imgui_extra_math.h",
        "imgui_extra_math.inl",
        "imgui_node_editor.h",
        "imgui_node_editor_internal.h",
        "imgui_node_editor_internal.inl",
    ],
    deps = ["@imgui"],
)
