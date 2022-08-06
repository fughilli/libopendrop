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
    deps = [
        "@gl",
    ],
)

cc_library(
    name = "glm_helper",
    hdrs = ["glm_helper.h"],
)
