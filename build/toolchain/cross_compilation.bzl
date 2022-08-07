_PI_VIDEOCORE_COPTS = [
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include",
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include/interface/vcos/pthreads",
    "-isystem",
    "external/raspberry_pi/sysroot/opt/vc/include/interface/vmcs_host/linux",
    "-L",
    "external/raspberry_pi/sysroot/opt/vc/lib",
]

_PI_SYSROOT_COPTS = [
    "-isystem",
    "external/raspberry_pi/sysroot/usr/include",
    "-isystem",
    "external/raspberry_pi/sysroot/usr/include/SDL2",
]

COPTS = select({
    "//:pi_build": _PI_VIDEOCORE_COPTS + _PI_SYSROOT_COPTS,
    "//:clang_build": [],
    "//conditions:default": [],
})

LINKOPTS = select({
    "//:pi_build": [
        "-Wl,-z,notext",
        "-lSDL2",
        "-lasound",
        "-lpulse",
        "-lX11",
        "-lXext",
        "-lXt",
        "-lsndio",
        "-lXcursor",
        "-lXinerama",
        "-lXrandr",
        "-lwayland-cursor",
        "-lwayland-client",
        "-lwayland-egl",
        "-lxkbcommon",
        "-lXxf86vm",
        "-lXss",
        "-lXi",
        "-lGL",
        "-Lexternal/raspberry_pi/sysroot/opt/vc/lib",
        "-lbcm_host",
    ],
    "//:clang_build": [
    ],
    "//conditions:default": [],
})

DEPS = select({
    "//:pi_build": ["@org_llvm_libcxx//:libcxx"],
    "//:clang_build": [
        "@org_llvm_libcxx//:libcxx",
        "//third_party:gl_helper",
        "//third_party:sdl_helper",
    ],
    "//conditions:default": [],
})
