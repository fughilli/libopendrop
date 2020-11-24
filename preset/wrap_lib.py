import hashlib
import os
from typing import Text

from absl import flags

FLAGS = flags.FLAGS

flags.DEFINE_string(
    "outputs", None,
    "Output header and source filenames, separated by a space")


def MakeHashString(input_filename: Text, code_text: Text):
    return hashlib.sha1(
        (input_filename + code_text).encode('utf-8')).hexdigest()


def MakeGuardSymbol(input_filename: Text) -> Text:
    return '{}_H_'.format(
        input_filename.translate(str.maketrans('.-/', '___')).upper())


def MakeNamespace(input_filename: Text) -> Text:
    return os.path.basename(input_filename).translate(
        str.maketrans('.-', '__')).lower()


def GetHeaderFilename(outputs: Text) -> Text:
    return outputs.split(' ')[0]


def GetSourceFilename(outputs: Text) -> Text:
    return outputs.split(' ')[1]
