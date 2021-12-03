ShaderInfo = provider(doc = "", fields = ["headers"])

def _shader_library_impl(ctx):
    shader_files = depset(
        ctx.files.srcs,
    )
    shader_includes = depset(
        ctx.files.hdrs,
        transitive = [dep[ShaderInfo].headers for dep in ctx.attr.deps],
    )

    if (len(ctx.files.srcs) >= 2):
        fail("shader_library does not support multiple srcs.")

    providers = [
        ShaderInfo(headers = depset(shader_includes)),
    ]

    if (len(ctx.files.srcs) != 0):
        source = ctx.files.srcs[0]
        header_file = ctx.outputs.outs[0]
        source_file = ctx.outputs.outs[1]
        args = ctx.actions.args()
        args.add("--shader_filename", source)
        args.add_joined(
            "--outputs",
            [header_file, source_file],
            join_with = " ",
        )

        ctx.actions.run(
            mnemonic = "WrapShader",
            inputs = depset(ctx.files.srcs, transitive = [shader_includes]),
            outputs = [header_file, source_file],
            executable = ctx.executable._wrap_shader_tool,
            arguments = [args],
        )
        providers.append(
            DefaultInfo(files = depset([header_file, source_file])),
        )

    return providers

shader_library = rule(
    implementation = _shader_library_impl,
    attrs = {
        "srcs": attr.label_list(allow_files = [".vsh", ".fsh"]),
        "hdrs": attr.label_list(allow_files = [".shh"]),
        "deps": attr.label_list(providers = [DefaultInfo]),
        "outs": attr.output_list(),
        "_wrap_shader_tool": attr.label(
            default = Label("//libopendrop/preset:wrap_shader"),
            executable = True,
            cfg = "exec",
        ),
    },
)

def shader_cc_library(name, srcs = [], hdrs = [], deps = []):
    if (len(srcs) == 0):
        shader_library(
            name = (name + "_gen"),
            srcs = srcs,
            hdrs = hdrs,
            outs = [],
            deps = [dep + "_gen" for dep in deps],
        )

    if (len(srcs) == 1):
        shader_source = srcs[0]
        header_file = shader_source + ".h"
        source_file = shader_source + ".cc"

        shader_library(
            name = (name + "_gen"),
            srcs = srcs,
            hdrs = hdrs,
            outs = [header_file, source_file],
            deps = [dep + "_gen" for dep in deps],
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
