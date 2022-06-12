# _PI_VIDEOCORE_COPTS = [
#     "-isystem",
#     "external/raspberry_pi/sysroot/opt/vc/include",
#     "-isystem",
#     "external/raspberry_pi/sysroot/opt/vc/include/interface/vcos/pthreads",
#     "-isystem",
#     "external/raspberry_pi/sysroot/opt/vc/include/interface/vmcs_host/linux",
#     "-L",
#     "external/raspberry_pi/sysroot/opt/vc/lib",
# ]
#
# _PI_SYSROOT_COPTS = [
#     "-isystem",
#     "external/raspberry_pi/sysroot/usr/include",
# ]
#
# COPTS = select({
#     "//:pi_build": _PI_VIDEOCORE_COPTS + _PI_SYSROOT_COPTS,
#     "//:clang_build": [],
#     "//conditions:default": [],
# })
#
# LINKOPTS = select({
#     "//:pi_build": [
#         "-Lexternal/raspberry_pi/sysroot/opt/vc/lib",
#         "-lbcm_host",
#     ],
#     "//:clang_build": [
#     ],
#     "//conditions:default": [],
# })
#
# DEPS = select({
#     "//:pi_build": ["@org_llvm_libcxx//:libcxx"],
#     "//:clang_build": ["@org_llvm_libcxx//:libcxx"],
#     "//conditions:default": [],
# })

COPTS = []
LINKOPTS = []
DEPS = []
