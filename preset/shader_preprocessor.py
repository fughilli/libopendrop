import re
import functools
from typing import List, Tuple, Text, Dict, Any

Symbols = Dict[Text, Any]

DEFINE_RE = re.compile(
    "#define (?P<symbol>[A-Za-z_][A-Za-z9-9_]*)( (?P<definition>.*))?")
UNDEF_RE = re.compile("#undef (?P<symbol>[A-Za-z_][A-Za-z9-9_]*)")
IFDEF_RE = re.compile(
    "#if(?P<negate>n)?ndef (?P<symbol>[A-Za-z_][A-Za-z9-9_]*)")
ENDIF_RE = re.compile("#endif.*")
INCLUDE_RE = re.compile(
    "#include \"(?P<filename>[A-Za-z0-9_][A-Za-z0-9/_]+)\"")


def MaybeHandleDefine(line: Text, symbols: Symbols) -> bool:
    match = DEFINE_RE.match(line)
    if match is None:
        return False

    symbol = match.groupdict()["symbol"]
    definition = match.groupdict()["definition"]

    symbols[symbol] = None
    return True


def MaybeHandleUndef(line: Text, symbols: Symbols) -> bool:
    match = UNDEF_RE.match(line)
    if match is None:
        return False

    symbol = match.groupdict()["symbol"]
    if symbol in symbols:
        del symbols[symbol]
        return True

    raise ValueError(
        "Can't undefine already undefined symbol \"{}\"".format(symbol))


class Source(object):
    def __init__(self):
        self.text = ""
        self.if_clauses = []

    @classmethod
    def Load(cls, source_filename: Text, symbols: Symbols):
        source = cls()
        with open(source_filename, "r") as source_file:
            for line in source_file.readlines():
                if MaybeHandleDefine(line, symbols):
                    continue
                if MaybeHandleUndef(line, symbols):
                    continue
                if source.MaybeHandleInclude(line, symbols):
                    continue
                if source.MaybeHandleIfdef(line, symbols):
                    continue
                if source.MaybeHandleEndif(line):
                    continue

                if source.IsLineEnabled():
                    source.text += line
        return source

    def MaybeHandleInclude(self, line: Text, symbols: Symbols) -> bool:
        match = INCLUDE_RE.match(line)
        if match is None:
            return False

        filename = match.groupdict()["filename"]

        self.text += Source.Load(filename, symbols).Render()
        return True

    def MaybeHandleIfdef(self, line: Text, symbols: Symbols) -> bool:
        match = IFDEF_RE.match(line)
        if match is None:
            return False

        symbol = match.groupdict()["symbol"]
        negated = "negate" in match.groupdict().keys()
        defined = symbol in symbols.keys()

        enabled = (not defined) if negated else defined
        self.if_clauses.append(enabled)
        return True

    def IsLineEnabled(self) -> bool:
        return functools.reduce((lambda a, b: a and b), self.if_clauses, True)

    def MaybeHandleEndif(self, line: Text) -> bool:
        match = ENDIF_RE.match(line)
        if match is None:
            return False

        if len(self.if_clauses) == 0:
            raise ValueError("No matching #if for #endif")

        self.if_clauses = self.if_clauses[:-1]
        return True

    def Render(self) -> Text:
        return self.text
