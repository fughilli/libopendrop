package(default_visibility = ["//visibility:public"])

cc_library(
    name = "oneshot",
    hdrs = ["oneshot.h"],
)

cc_library(
    name = "periodic",
    hdrs = ["periodic.h"],
)

cc_library(
    name = "rate_limiter",
    hdrs = ["rate_limiter.h"],
    deps = [":oneshot"],
)

cc_library(
    name = "performance_timer",
    hdrs = ["performance_timer.h"],
)
