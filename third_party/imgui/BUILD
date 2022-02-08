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
    linkopts = [
        "-lm",
        # FIXME(cfrantz): Detect whether we're building for linux or windows,
        # and select the appropriate GL library
        # "-lGL",
        # "-lopengl32",
    ],
)

cc_library(
    name = "imgui_backend_sdl",
    srcs = ["backends/imgui_impl_sdl.cpp"],
    hdrs = ["backends/imgui_impl_sdl.h"],
    deps = [":imgui"],
)

cc_library(
    name = "imgui_backend_opengl2",
    srcs = ["backends/imgui_impl_opengl2.cpp"],
    hdrs = ["backends/imgui_impl_opengl2.h"],
    deps = [":imgui"],
)
