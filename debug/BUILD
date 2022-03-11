package(default_visibility = ["//visibility:public"])

load("@com_google_protobuf//:protobuf.bzl", "py_proto_library")

cc_library(
    name = "proto_port",
    hdrs = ["proto_port.h"],
    deps = ["//util:udp_server"],
)

cc_library(
    name = "control_injector",
    hdrs = ["control_injector.h"],
    deps = [
        ":control_cc_proto",
        ":proto_port",
        "//util:logging",
        "//util:math",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@implot",
    ],
)

cc_library(
    name = "signal_scope",
    hdrs = ["signal_scope.h"],
    deps = [
        "//util:logging",
        "//util:math",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@implot",
    ],
)

proto_library(
    name = "control_proto",
    srcs = ["control.proto"],
)

cc_proto_library(
    name = "control_cc_proto",
    deps = [":control_proto"],
)

py_proto_library(
    name = "control_py_proto",
    srcs = ["control.proto"],
)

py_binary(
    name = "control_sender",
    srcs = ["control_sender.py"],
    deps = [":control_py_proto"],
)
