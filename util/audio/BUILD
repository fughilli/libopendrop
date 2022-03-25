package(default_visibility = ["//visibility:public"])

cc_library(
    name = "pulseaudio_interface",
    srcs = ["pulseaudio_interface.cc"],
    hdrs = ["pulseaudio_interface.h"],
    linkopts = [
        "-lpulse",
    ],
    linkstatic = 1,
    deps = [
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "normalizer",
    hdrs = ["normalizer.h"],
    deps = [
        "//debug:signal_scope",
        "//util/logging",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "audio_processor",
    srcs = ["audio_processor.cc"],
    hdrs = ["audio_processor.h"],
    linkstatic = 1,
    deps = [
        "@com_google_absl//absl/types:span",
    ],
)
