load("@py_deps//:requirements.bzl", "requirement")

package(default_visibility = ["//:__subpackages__"])

py_binary(
    name = "filter_explorer",
    srcs = ["filter_explorer.py"],
    deps = [
        requirement("matplotlib"),
        requirement("numpy"),
    ],
)

py_binary(
    name = "wrap_shader",
    srcs = ["wrap_shader.py"],
    python_version = "PY3",
    deps = [
        requirement("absl-py"),
    ],
)

py_test(
    name = "wrap_shader_test",
    srcs = ["wrap_shader_test.py"],
    deps = [
        ":wrap_shader",
        requirement("pyfakefs"),
        requirement("parameterized"),
    ],
)

py_binary(
    name = "wrap_model",
    srcs = ["wrap_model.py"],
    python_version = "PY3",
    deps = [
        requirement("absl-py"),
        requirement("numpy"),
    ],
)

py_test(
    name = "wrap_model_test",
    srcs = ["wrap_model_test.py"],
    deps = [
        ":wrap_model",
        requirement("parameterized"),
    ],
)
