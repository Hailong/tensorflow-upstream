# Description:
#   ROCm-platform specific StreamExecutor support code.

licenses(["notice"])  # Apache 2.0

load("//tensorflow:tensorflow.bzl", "tf_cc_test")
load(
    "//tensorflow/stream_executor:build_defs.bzl",
    "stream_executor_friends",
)
load("//tensorflow:tensorflow.bzl", "tf_copts")
load("@local_config_rocm//rocm:build_defs.bzl", "if_rocm_is_configured")
load("//tensorflow/core:platform/default/build_config_root.bzl", "if_static")

package_group(
    name = "friends",
    packages = stream_executor_friends(),
)

package(
    default_visibility = [":friends"],
)

# Filegroup used to collect source files for the dependency check.
filegroup(
    name = "c_srcs",
    data = glob([
        "**/*.cc",
        "**/*.h",
    ]),
)

cc_library(
    name = "rocm_diagnostics",
    srcs = if_rocm_is_configured(["rocm_diagnostics.cc"]),
    hdrs = [],
    deps = if_rocm_is_configured([
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "//tensorflow/stream_executor/gpu:gpu_diagnostics_header",
        "//tensorflow/stream_executor/lib",
        "//tensorflow/stream_executor/platform",
    ]),
)

cc_library(
    name = "rocm_driver",
    srcs = if_rocm_is_configured(["rocm_driver.cc"]),
    hdrs = [],
    deps = if_rocm_is_configured([
        ":rocm_diagnostics",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/strings",
        "//tensorflow/stream_executor:device_options",
        "//tensorflow/stream_executor/gpu:gpu_driver_header",
        "//tensorflow/stream_executor/lib",
        "//tensorflow/stream_executor/platform",
        "@local_config_rocm//rocm:rocm_headers",
    ]),
)

cc_library(
    name = "rocm_event",
    srcs = if_rocm_is_configured(["rocm_event.cc"]),
    hdrs = [],
    deps = if_rocm_is_configured([
        ":rocm_driver",
        "//tensorflow/stream_executor:stream_executor_headers",
        "//tensorflow/stream_executor/gpu:gpu_event_header",
        "//tensorflow/stream_executor/gpu:gpu_executor_header",
        "//tensorflow/stream_executor/gpu:gpu_stream_header",
        "//tensorflow/stream_executor/lib",
    ]),
)

cc_library(
    name = "rocm_gpu_executor",
    srcs = if_rocm_is_configured(["rocm_gpu_executor.cc"]),
    hdrs = [],
    deps = if_rocm_is_configured([
        ":rocm_diagnostics",
        ":rocm_driver",
        ":rocm_event",
        ":rocm_kernel",
        ":rocm_platform_id",
        "@com_google_absl//absl/strings",
        "//tensorflow/stream_executor:event",
        "//tensorflow/stream_executor:plugin_registry",
        "//tensorflow/stream_executor:stream_executor_internal",
        "//tensorflow/stream_executor:stream_executor_pimpl_header",
        "//tensorflow/stream_executor:timer",
        "//tensorflow/stream_executor/gpu:gpu_activation_header",
        "//tensorflow/stream_executor/gpu:gpu_event",
        "//tensorflow/stream_executor/gpu:gpu_kernel_header",
        "//tensorflow/stream_executor/gpu:gpu_stream",
        "//tensorflow/stream_executor/gpu:gpu_timer",
        "//tensorflow/stream_executor/lib",
        "//tensorflow/stream_executor/platform",
        "//tensorflow/stream_executor/platform:dso_loader",
    ]),
    alwayslink = True,
)

cc_library(
    name = "rocm_kernel",
    srcs = if_rocm_is_configured(["rocm_kernel.cc"]),
    hdrs = [],
    visibility = ["//visibility:public"],
    deps = if_rocm_is_configured([
        "//tensorflow/stream_executor/gpu:gpu_kernel_header",
    ]),
    alwayslink = True,
)

cc_library(
    name = "rocm_platform",
    srcs = if_rocm_is_configured(["rocm_platform.cc"]),
    hdrs = if_rocm_is_configured(["rocm_platform.h"]),
    visibility = ["//visibility:public"],
    deps = if_rocm_is_configured([
        ":rocm_driver",
        ":rocm_gpu_executor",
        ":rocm_platform_id",
        "//tensorflow/stream_executor",  # buildcleaner: keep
        "//tensorflow/stream_executor:executor_cache",
        "//tensorflow/stream_executor:multi_platform_manager",
        "//tensorflow/stream_executor:stream_executor_pimpl_header",
        "//tensorflow/stream_executor/lib",
        "//tensorflow/stream_executor/platform",
    ]),
    alwayslink = True,  # Registers itself with the MultiPlatformManager.
)

cc_library(
    name = "rocm_platform_id",
    srcs = ["rocm_platform_id.cc"],
    hdrs = ["rocm_platform_id.h"],
    deps = ["//tensorflow/stream_executor:platform"],
)

# FIXME: enable in future PRs
#cc_library(
#    name = "rocblas_plugin",
#    srcs = ["rocm_blas.cc"],
#    hdrs = ["rocm_blas.h"],
#    visibility = ["//visibility:public"],
#    deps = [
#        ":rocm_gpu_executor",
#        ":rocm_platform_id",
#        "//third_party/eigen3",
#        "//tensorflow/core:lib_internal",
#        "//tensorflow/stream_executor",
#        "//tensorflow/stream_executor:event",
#        "//tensorflow/stream_executor:host_or_device_scalar",
#        "//tensorflow/stream_executor:plugin_registry",
#        "//tensorflow/stream_executor:scratch_allocator",
#        "//tensorflow/stream_executor:timer",
#        "//tenosrflow/stream_executor/gpu:gpu_activation_header",
#        "//tenosrflow/stream_executor/gpu:gpu_stream_header",
#        "//tenosrflow/stream_executor/gpu:gpu_timer_header",
#        "//tensorflow/stream_executor/lib",
#        "//tensorflow/stream_executor/platform",
#        "//tensorflow/stream_executor/platform:dso_loader",
#        "@com_google_absl//absl/strings",
#        "@local_config_rocm//rocm:rocm_headers",
#    ] + if_static(["@local_config_rocm//rocm:rocblas"]),
#    alwayslink = True,
#)

# FIXME: enable in future PRs
#cc_library(
#    name = "rocfft_plugin",
#    srcs = ["rocm_fft.cc"],
#    hdrs = [],
#    visibility = ["//visibility:public"],
#    deps = [
#        ":rocm_platform_id",
#        "//tensorflow/stream_executor:event",
#        "//tensorflow/stream_executor:fft",
#        "//tensorflow/stream_executor:plugin_registry",
#        "//tensorflow/stream_executor:scratch_allocator",
#        "//tenosrflow/stream_executor/gpu:gpu_stream_header",
#        "//tensorflow/stream_executor/lib",
#        "//tensorflow/stream_executor/platform",
#        "//tensorflow/stream_executor/platform:dso_loader",
#        "@local_config_rocm//rocm:rocm_headers",
#    ] + if_static(["@local_config_rocm//rocm:rocfft"]),
#    alwayslink = True,
#)

# FIXME: enable in future PRs
#cc_library(
#    name = "miopen_plugin",
#    srcs = ["rocm_dnn.cc"],
#    hdrs = [],
#    copts = [
#        # STREAM_EXECUTOR_CUDNN_WRAP would fail on Clang with the default
#        # setting of template depth 256
#        "-ftemplate-depth-512",
#    ],
#    visibility = ["//visibility:public"],
#    deps = [
#        ":rocm_diagnostics",
#        ":rocm_driver",
#        ":rocm_gpu_executor",
#        ":rocm_platform_id",
#        "//third_party/eigen3",
#        "//tensorflow/core:lib",
#        "//tensorflow/core:lib_internal",
#        "//tensorflow/core:logger",
#        "//tensorflow/stream_executor:dnn",
#        "//tensorflow/stream_executor:event",
#        "//tensorflow/stream_executor:logging_proto_cc",
#        "//tensorflow/stream_executor:plugin_registry",
#        "//tensorflow/stream_executor:scratch_allocator",
#        "//tensorflow/stream_executor:stream_executor_pimpl_header",
#        "//tensorflow/stream_executor:temporary_device_memory",
#        "//tenosrflow/stream_executor/gpu:gpu_activation_header",
#        "//tenosrflow/stream_executor/gpu:gpu_stream_header",
#        "//tenosrflow/stream_executor/gpu:gpu_timer_header",
#        "//tensorflow/stream_executor/lib",
#        "//tensorflow/stream_executor/platform",
#        "//tensorflow/stream_executor/platform:dso_loader",
#        "@com_google_absl//absl/strings",
#        "@local_config_rocm//rocm:rocm_headers",
#    ] + tf_additional_miopen_plugin_deps() + if_static(["@local_config_rocm//rocm:miopen"]),
#    alwayslink = True,
#)

# FIXME: enable in future PRs
#cc_library(
#    name = "rocrand_plugin",
#    srcs = ["rocm_rng.cc"],
#    hdrs = [],
#    deps = [
#        ":rocm_gpu_executor",
#        ":rocm_platform_id",
#        "@local_config_rocm//rocm:rocm_headers",
#        "//tensorflow/stream_executor:event",
#        "//tensorflow/stream_executor:plugin_registry",
#        "//tensorflow/stream_executor:rng",
#        "//tenosrflow/stream_executor/gpu:gpu_activation_header",
#        "//tenosrflow/stream_executor/gpu:gpu_stream_header",
#        "//tensorflow/stream_executor/lib",
#        "//tensorflow/stream_executor/platform",
#        "//tensorflow/stream_executor/platform:dso_loader",
#    ] + if_static(["@local_config_rocm//rocm:curand"]),
#    alwayslink = True,
#)

cc_library(
    name = "all_runtime",
    copts = tf_copts(),
    visibility = ["//visibility:public"],
    deps = if_rocm_is_configured([
        # FIXME: enable in future PRs
        #":miopen_plugin",
        #":rocfft_plugin",
        #":rocblas_plugin",
        #":rocrand_plugin",
        ":rocm_driver",
        ":rocm_platform",
    ]),
    alwayslink = 1,
)