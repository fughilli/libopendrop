package(default_visibility = ["//libopendrop:__subpackages__"])

PRESET_DEPS_LIST = [
    "//libopendrop/preset/alien_rorschach",
    "//libopendrop/preset/cube_boom",
    "//libopendrop/preset/glowsticks_3d",
    "//libopendrop/preset/glowsticks_3d_zoom",
    "//libopendrop/preset/kaleidoscope",
    "//libopendrop/preset/rotary_transporter",
    "//libopendrop/preset/simple_preset",
    "//libopendrop/preset/template_preset",
    "//libopendrop/preset/shape_bounce",
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
        "@com_google_absl//absl/status:statusor",
        "//libopendrop/util:status_macros",
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
    python_version = "PY3",
)

py_binary(
    name = "wrap_model",
    srcs = ["wrap_model.py"],
    python_version = "PY3",
)

py_test(
    name = "wrap_model_test",
    srcs = ["wrap_model_test.py"],
    deps = [
        ":wrap_model",
    ],
)
