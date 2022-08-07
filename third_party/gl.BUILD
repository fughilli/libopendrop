package(default_visibility = ["//visibility:public"])

cc_library(
    name = "gl",
    srcs = ["lib/x86_64-linux-gnu/libGL.so"],
    hdrs = glob(["include/GL/**/*.h"]),
    includes = [
        "include",
        "include/GL",
    ],
)
