load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
    CROSS_COMPILATION_LINKOPTS = "LINKOPTS",
)

package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "graph_builtins",
    srcs = ["graph_builtins.cc"],
    hdrs = ["graph_builtins.h"],
    deps = [
        ":graph",
        "//util/graph/types:texture",
    ],
)

cc_library(
    name = "tuple",
    srcs = ["tuple.cc"],
    hdrs = ["tuple.h"],
    deps = [
        "//util/graph/types",
        "//util/logging",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "tuple_factory",
    hdrs = ["tuple_factory.h"],
    deps = [
        ":tuple",
        "//util/graph/types",
        "//util/graph/types:opaque_storable",
        "@com_google_absl//absl/container:flat_hash_map",
        "//util/graph/types:monotonic",
        "//util/graph/types:unitary",
        "//util/graph/types:texture",
    ],
)

cc_test(
    name = "tuple_test",
    srcs = ["tuple_test.cc"],
    deps = [
        ":tuple",
        "@com_googletest//:gtest",
        "//util/graph/types",
        "//util/graph/types:monotonic",
        "//util/graph/types:unitary",
        "//util/testing:test_main",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "conversion",
    srcs = ["conversion.cc"],
    hdrs = ["conversion.h"],
    deps = [
        ":tuple",
        ":tuple_factory",
        "//util/graph/types",
        "//util/logging",
    ],
)

cc_test(
    name = "conversion_test",
    srcs = ["conversion_test.cc"],
    deps = [
        ":conversion",
        ":tuple_factory",
        "@com_googletest//:gtest",
        "//util/testing:test_main",
        "//util/graph/types",
        "//util/graph/types:monotonic",
        "//util/graph/types:unitary",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "graph",
    srcs = ["graph.cc"],
    hdrs = ["graph.h"],
    deps = [
        ":conversion",
        ":tuple",
        "//third_party:gl_helper",
        "//util/container:algorithms",
        "//util/graph/types",
        "//util/logging",
        "//util/status:status_macros",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "graph_test",
    srcs = ["graph_test.cc"],
    linkopts = [
    ] + CROSS_COMPILATION_LINKOPTS,
    deps = [
        "//util/math",
        "//util/graphics:colors",
        "//util/graphics:gl_util",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        ":graph",
        "@com_google_absl//absl/strings",
        "@com_googletest//:gtest",
        "//util/testing:test_main",
        "//util/logging",
        ":graph_builtins",
        "//util/testing:graph_matchers",
        "//util/graph/types",
        "//util/graph/types:monotonic",
        "//util/graph/types:unitary",
        "//util/graph/types:color",
    ] + CROSS_COMPILATION_DEPS,
)

cc_library(
    name = "rendering",
    srcs = ["rendering.cc"],
    hdrs = ["rendering.h"],
    deps = [
        ":conversion",
        ":graph",
        "//util/container:algorithms",
        "@com_google_absl//absl/types:span",
        "@imgui",
        "@imgui_node_editor",
    ],
)
