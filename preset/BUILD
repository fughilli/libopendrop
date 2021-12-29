package(default_visibility = ["//:__subpackages__"])

load("@py_deps//:requirements.bzl", "requirement")

PRESET_DEPS_LIST = [
    "//preset/alien_rorschach",
    "//preset/cube_boom",
    "//preset/glowsticks_3d",
    "//preset/glowsticks_3d_zoom",
    "//preset/kaleidoscope",
    "//preset/rotary_transporter",
    "//preset/simple_preset",
    "//preset/template_preset",
    "//preset/shape_bounce",
    "//preset/eye_roll",
    "//preset/cube_wreath",
]

cc_library(
    name = "preset",
    srcs = ["preset.cc"],
    hdrs = ["preset.h"],
    linkstatic = 1,
    deps = [
        "//:gl_interface",
        "//:gl_render_target",
        "//:global_state",
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
        "//util:status_macros",
    ] + PRESET_DEPS_LIST,
)

cc_library(
    name = "preset_blender",
    srcs = ["preset_blender.cc"],
    hdrs = ["preset_blender.h"],
    deps = [
        ":preset",
        "//:blit_fsh",
        "//:blit_vsh",
        "//primitive:rectangle",
        "//util:gl_util",
        "//util:logging",
        "//util:oneshot",
    ],
)

py_binary(
    name = "wrap_shader",
    srcs = ["wrap_shader.py"],
    python_version = "PY3",
    deps = [
        requirement("absl-py"),
    ],
)

py_test(
    name = "wrap_shader_test",
    srcs = ["wrap_shader_test.py"],
    deps = [
        ":wrap_shader",
        requirement("pyfakefs"),
    ],
)

py_binary(
    name = "wrap_model",
    srcs = ["wrap_model.py"],
    python_version = "PY3",
    deps = [
        requirement("absl-py"),
    ],
)

py_test(
    name = "wrap_model_test",
    srcs = ["wrap_model_test.py"],
    deps = [
        ":wrap_model",
    ],
)
