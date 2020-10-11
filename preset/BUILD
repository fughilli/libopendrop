package(default_visibility = ["//libopendrop:__subpackages__"])

PRESET_DEPS_LIST = [
    "//libopendrop/preset/kaleidoscope",
    "//libopendrop/preset/simple_preset",
    "//libopendrop/preset/alien_rorschach",
    "//libopendrop/preset/template_preset",
]

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
    name = "preset_list",
    hdrs = ["preset_list.h"],
    linkstatic = 1,
    deps = [
        ":preset",
    ] + PRESET_DEPS_LIST,
)

cc_library(
    name = "preset_blender",
    srcs = ["preset_blender.cc"],
    hdrs = ["preset_blender.h"],
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
