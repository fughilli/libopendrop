package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "constants",
    hdrs = ["constants.h"],
    srcs = ["constants.cc"],
)

cc_library(
    name = "perspective",
    srcs = ["perspective.cc"],
    hdrs = ["perspective.h"],
)

cc_library(
    name = "vector",
    srcs = ["vector.cc"],
    hdrs = ["vector.h"],
    deps = [
        ":constants",
        "//util:logging",
    ],
)
