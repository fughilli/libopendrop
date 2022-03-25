package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "glm_matchers",
    hdrs = ["glm_matchers.h"],
    deps = [
        "//third_party:glm_helper",
        "@com_googletest//:gtest",
    ],
)

cc_library(
    name = "re2_matchers",
    hdrs = ["re2_matchers.h"],
    deps = [
        "@com_google_absl//absl/strings:str_format",
        "@com_google_re2//:re2",
        "@com_googletest//:gtest",
    ],
)
