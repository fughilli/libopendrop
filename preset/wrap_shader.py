import hashlib
import os
import re
from typing import Text, List, Tuple

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

static inline const char* Code() {{
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

INCLUDE_RE = re.compile(r"#include (<|\")(?P<include>[a-zA-Z0-9-._\/]+)(>|\")")
DEFINE_RE = re.compile(
    r"#(?P<type>define|undef|ifdef|ifndef) (?P<symbol>[a-zA-Z0-9_]+)")
ENDIF_RE = re.compile(r"#endif\s*(//.*)?")


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


def LoadInclude(filename: Text, env: List[Text]) -> Text:
    with open(filename, 'r') as include_file:
        return ProcessMacros(filename, include_file.read(), env)


def AcceptLine(processing_stack: List[Tuple[Text, bool]]):
    for _, active in processing_stack:
        if not active:
            return False
    return True


def ProcessMacros(filename: Text, code_text: Text, env: List[Text]) -> Text:
    output_text_lines: List[Text] = []
    processing_stack: List[Tuple[Text, bool]] = []
    for line in code_text.split('\n'):
        include_match = INCLUDE_RE.match(line)
        if include_match:
            output_text_lines.append(
                LoadInclude(include_match.groupdict()["include"], env).strip())
            continue

        define_match = DEFINE_RE.match(line)
        if define_match:
            define_type = define_match.groupdict()["type"]
            symbol = define_match.groupdict()["symbol"]
            if define_type == "define":
                if symbol not in env:
                    env.append(symbol)
                    continue
            elif define_type == "undef":
                if symbol in env:
                    env.remove(symbol)
                    continue
                raise Exception(f"{filename:s}:{line_number:d}: "
                                f"Can't #undef undefined symbol {symbol:s}")
            elif define_type == "ifdef":
                processing_stack.append((symbol, (symbol in env)))
                continue
            elif define_type == "ifndef":
                processing_stack.append((symbol, (symbol not in env)))
                continue
            else:
                raise Exception(f"{filename:s}:{line_number:d}: "
                                f"Unknown macro #{define_type:s}")

        endif_match = ENDIF_RE.match(line)
        if endif_match:
            if len(processing_stack) == 0:
                raise Exception(f"{filename:s}:{line_number:d}: "
                                f"No matching #if for #endif")
            processing_stack.pop()
            continue

        if AcceptLine(processing_stack):
            output_text_lines.append(line)

    return '\n'.join(output_text_lines)


def main(argv):
    if FLAGS.shader_filename == None:
        raise ValueError("--shader_filename must be specified")

    print("Absolute path:", os.path.abspath(os.curdir))
    print("Input:", FLAGS.shader_filename)

    shader_filename = FLAGS.shader_filename

    code_text = open(shader_filename, 'r').read()

    env: List[Text] = []
    code_text = ProcessMacros(shader_filename, code_text, env)

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
