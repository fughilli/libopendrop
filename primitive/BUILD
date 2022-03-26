package(default_visibility = ["//:__subpackages__"])

cc_library(
    name = "primitive",
    hdrs = ["primitive.h"],
)

cc_library(
    name = "polyline",
    srcs = ["polyline.cc"],
    hdrs = ["polyline.h"],
    deps = [
        ":primitive",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "rectangle",
    srcs = ["rectangle.cc"],
    hdrs = ["rectangle.h"],
    deps = [
        ":primitive",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
    ],
)

cc_library(
    name = "ngon",
    srcs = ["ngon.cc"],
    hdrs = ["ngon.h"],
    deps = [
        ":primitive",
        "//third_party:gl_helper",
    ],
)

cc_library(
    name = "ribbon",
    hdrs = [
        "ribbon.cc",
        "ribbon.h",
    ],
    deps = [
        ":primitive",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "//util/logging",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "model",
    srcs = ["model.cc"],
    hdrs = ["model.h"],
    deps = [
        ":primitive",
        "//third_party:gl_helper",
        "//third_party:glm_helper",
        "@com_google_absl//absl/types:span",
    ],
)
