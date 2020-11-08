def shader_cc_library(name, srcs):
    if len(srcs) != 1:
        fail("srcs must have a single element")

    shader_source = srcs[0]
    wrap_shader_tool = "//libopendrop/preset:wrap_shader"

    header_file = shader_source + ".h"
    source_file = shader_source + ".cc"

    native.genrule(
        name = (name + "_gen"),
        srcs = [shader_source],
        outs = [header_file, source_file],
        cmd = (("$(location %s) --shader_filename $(SRCS) --outputs " +
                "\"$(OUTS)\"") % (wrap_shader_tool,)),
        tools = [wrap_shader_tool],
    )

    native.cc_library(
        name = name,
        srcs = [source_file],
        hdrs = [header_file],
    )

def model_cc_library(name, srcs):
    if len(srcs) != 1:
        fail("srcs must have a single element")

    model_source = srcs[0]
    wrap_model_tool = "//libopendrop/preset:wrap_model"

    header_file = model_source + ".h"
    source_file = model_source + ".cc"

    native.genrule(
        name = (name + "_gen"),
        srcs = [model_source],
        outs = [header_file, source_file],
        cmd = (("$(location %s) --object_filename $(SRCS) --outputs " +
                "\"$(OUTS)\"") % (wrap_model_tool,)),
        tools = [wrap_model_tool],
    )

    native.cc_library(
        name = name,
        srcs = [source_file],
        hdrs = [header_file],
    )
