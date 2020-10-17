package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "logging",
    hdrs = ["logging.h"],
    linkstatic = 1,
)

cc_library(
    name = "colors",
    srcs = ["colors.cc"],
    hdrs = ["colors.h"],
    linkstatic = 1,
)

cc_library(
    name = "gl_util",
    srcs = ["gl_util.cc"],
    hdrs = ["gl_util.h"],
    linkstatic = 1,
    deps = [
        "//libopendrop:gl_interface",
        "//libopendrop:gl_render_target",
    ],
)

cc_library(
    name = "oneshot",
    hdrs = ["oneshot.h"],
)

cc_library(
    name = "coefficients",
    hdrs = ["coefficients.h"],
)
