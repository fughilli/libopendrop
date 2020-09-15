import hashlib
import os
from typing import Text

from absl import app
from absl import flags

flags.DEFINE_string("shader_filename", None, "Input shader source file")
flags.DEFINE_string(
    "outputs", None,
    "Output header and source filenames, separated by a space")

FLAGS = flags.FLAGS

HEADER_TEMPLATE = ("""#ifndef {guard_symbol}
#define {guard_symbol}

namespace {namespace} {{

extern const char* __kCode_{hash_string};

inline const char* Code() {{
  return __kCode_{hash_string};
}}

}}

#endif // {guard_symbol}
""")

SOURCE_TEMPLATE = ("""namespace {namespace} {{

const char* __kCode_{hash_string} = R"(
{code_text}
)";

}}
""")


def MakeHashString(shader_filename: Text, code_text: Text):
    return hashlib.sha1(
        (shader_filename + code_text).encode('utf-8')).hexdigest()


def MakeGuardSymbol(shader_filename: Text) -> Text:
    return '{}_H_'.format(
        shader_filename.translate(str.maketrans('.-/', '___')).upper())


def MakeNamespace(shader_filename: Text) -> Text:
    return os.path.basename(shader_filename).translate(
        str.maketrans('.-', '__')).lower()


def GetHeaderFilename(outputs: Text) -> Text:
    return outputs.split(' ')[0]


def GetSourceFilename(outputs: Text) -> Text:
    return outputs.split(' ')[1]


def GenerateHeader(guard_symbol: Text, namespace: Text, code_text: Text,
                   hash_string: Text) -> Text:
    return HEADER_TEMPLATE.format(guard_symbol=guard_symbol,
                                  namespace=namespace,
                                  hash_string=hash_string)


def GenerateSource(namespace: Text, code_text: Text,
                   hash_string: Text) -> Text:
    return SOURCE_TEMPLATE.format(namespace=namespace,
                                  code_text=code_text,
                                  hash_string=hash_string)


def main(argv):
    if FLAGS.shader_filename == None:
        raise ValueError("--shader_filename must be specified")

    print("Absolute path:", os.path.abspath(os.curdir))
    print("Input:", FLAGS.shader_filename)

    shader_filename = FLAGS.shader_filename

    code_text = open(shader_filename, 'r').read()
    guard_symbol = MakeGuardSymbol(shader_filename)
    namespace = MakeNamespace(shader_filename)
    hash_string = MakeHashString(shader_filename, code_text)
    outputs = FLAGS.outputs

    header_filename = GetHeaderFilename(outputs)
    with open(header_filename, 'w') as header_file:
        header_file.write(
            GenerateHeader(guard_symbol=guard_symbol,
                           namespace=namespace,
                           code_text=code_text,
                           hash_string=hash_string))

    source_filename = GetSourceFilename(outputs)
    with open(source_filename, 'w') as source_file:
        source_file.write(
            GenerateSource(namespace=namespace,
                           code_text=code_text,
                           hash_string=hash_string))

    print("Wrote to", source_filename, "and", header_filename)


if __name__ == "__main__":
    app.run(main)
