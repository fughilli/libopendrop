package(default_visibility = ["//visibility:public"])

cc_library(
    name = "global_state",
    srcs = ["global_state.cc"],
    hdrs = ["global_state.h"],
    linkstatic = 1,
    deps = [
        "//util/signal:accumulator",
        "//util/signal:filter",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "open_drop_controller_interface",
    hdrs = ["open_drop_controller_interface.h"],
    deps = [
        "//util/audio:audio_processor",
        "//util/graphics:gl_interface",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "open_drop_controller",
    srcs = ["open_drop_controller.cc"],
    hdrs = ["open_drop_controller.h"],
    linkstatic = 1,
    deps = [
        ":global_state",
        ":open_drop_controller_interface",
        "//preset",
        "//preset:preset_blender",
        "//primitive:rectangle",
        "//shader:blit_fsh",
        "//shader:blit_vsh",
        "//util/audio:normalizer",
        "//util/graphics:gl_render_target",
        "//util/graphics:gl_util",
        "//util/logging",
        "@com_google_absl//absl/types:span",
    ],
)
