load(
    "//build/toolchain:cross_compilation.bzl",
    CROSS_COMPILATION_DEPS = "DEPS",
)

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "udp_client",
    srcs = ["udp_client.cc"],
    hdrs = ["udp_client.h"],
)

cc_library(
    name = "udp_server",
    srcs = ["udp_server.cc"],
    hdrs = ["udp_server.h"],
    deps = ["//util/logging"],
)

cc_test(
    name = "udp_test",
    srcs = ["udp_test.cc"],
    deps = [
        ":udp_client",
        ":udp_server",
        "@com_googletest//:gtest",
        "@com_googletest//:gtest_main",
    ] + CROSS_COMPILATION_DEPS,
)
