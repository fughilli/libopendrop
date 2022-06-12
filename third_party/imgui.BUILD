package(default_visibility = ["//visibility:public"])

cc_library(
    name = "imgui",
    srcs = [
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
    ],
    hdrs = [
        "imconfig.h",
        "imgui.h",
        "imgui_internal.h",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
    ],
)

cc_library(
    name = "imgui_backend_sdl",
    srcs = ["backends/imgui_impl_sdl.cpp"],
    hdrs = ["backends/imgui_impl_sdl.h"],
    deps = [
        ":imgui",
        "@sdl2",
    ],
)

cc_library(
    name = "imgui_backend_opengl2",
    srcs = ["backends/imgui_impl_opengl2.cpp"],
    hdrs = ["backends/imgui_impl_opengl2.h"],
    deps = [":imgui"],
)
