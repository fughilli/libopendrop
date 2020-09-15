package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "preset",
    srcs = ["preset.cc"],
    hdrs = ["preset.h"],
    linkstatic = 1,
    deps = [
        "//libopendrop:global_state",
        "@com_google_absl//absl/types:span",
    ],
)

py_binary(
    name = "wrap_shader",
    srcs = ["wrap_shader.py"],
)
