def model_cc_library(name, srcs, scale = None, normalize = None):
    if len(srcs) != 1:
        fail("srcs must have a single element")

    model_source = srcs[0]
    wrap_model_tool = "//tools:wrap_model"

    header_file = model_source + ".h"
    source_file = model_source + ".cc"

    scale_and_norm_args = ""
    if scale != None:
        scale_and_norm_args += "--scale=%f " % scale
    if normalize != None:
        scale_and_norm_args += "--normalize=%s " % (
            "true" if normalize else "false"
        )

    native.genrule(
        name = (name + "_gen"),
        srcs = [model_source],
        outs = [header_file, source_file],
        cmd = (("$(location %s) --object_filename $(SRCS) " +
                scale_and_norm_args + " --outputs \"$(OUTS)\"") % (
            wrap_model_tool,
        )),
        tools = [wrap_model_tool],
    )

    native.cc_library(
        name = name,
        srcs = [source_file],
        hdrs = [header_file],
        deps = ["@glm"],
    )
