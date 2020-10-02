package(default_visibility = ["//libopendrop:__subpackages__"])

cc_library(
    name = "preset",
    srcs = ["preset.cc"],
    hdrs = ["preset.h"],
    linkstatic = 1,
    deps = [
        "//libopendrop:gl_interface",
        "//libopendrop:gl_render_target",
        "//libopendrop:global_state",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "preset_blender",
    hdrs = ["preset_blender.h"],
    srcs = ["preset_blender.cc"],
    deps = [
        ":preset",
        "//libopendrop:blit_fsh",
        "//libopendrop:blit_vsh",
        "//libopendrop/primitive:rectangle",
        "//libopendrop/util:gl_util",
        "//libopendrop/util:logging",
        "//libopendrop/util:oneshot",
    ],
)

py_binary(
    name = "wrap_shader",
    srcs = ["wrap_shader.py"],
)
