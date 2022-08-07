package(default_visibility = ["//visibility:public"])

exports_files(
    [
        "implot.BUILD",
        "imgui.BUILD",
        "additional_deps.patch",
    ],
)

cc_library(
    name = "gl_helper",
    hdrs = ["gl_helper.h"],
    deps = select({
        "//:pi_build": [
            "@raspberry_pi//sysroot:gl",
        ],
        "//:clang_build": [
            "@host_gl//:gl",
        ],
    }),
)

cc_library(
    name = "sdl_helper",
    includes = select({
        "//:pi_build": [
        ],
        "//conditions:default": [],
    }),
    deps = select({
        "//:pi_build": [
            "@raspberry_pi//sysroot:sdl2",
        ],
        "//:clang_build": [
            "@host_sdl2//:sdl2",
        ],
    }),
)

cc_library(
    name = "glm_helper",
    hdrs = ["glm_helper.h"],
)
