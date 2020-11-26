import unittest
import textwrap
from pyfakefs.fake_filesystem_unittest import TestCase

from libopendrop.preset import shader_preprocessor

SIMPLE_SOURCE_TEXT = """
varying float f;

vec2 some_function() {
    return vec2(0.0, 0.0);
}
"""

SIMPLE_SOURCE_WITH_INCLUDES_TEXT = """
#include "foo/baz"
#include "foo/bar2"

varying float f;
"""


class ShaderPreprocessorTestCase(TestCase):
    def setUp(self):
        self.setUpPyfakefs()

    def test_load_empty_source_file_yields_empty(self):
        self.fs.create_file("/foo/bar", contents="")
        source = shader_preprocessor.Source.Load("/foo/bar", {})
        self.assertEqual(source.Render(), "")

    def test_load_simple_source_file(self):
        self.fs.create_file("/foo/bar", contents=SIMPLE_SOURCE_TEXT)
        source = shader_preprocessor.Source.Load("/foo/bar", {})
        self.assertEqual(source.Render(), SIMPLE_SOURCE_TEXT)

    def test_parse_define(self):
        symbols = {}
        shader_preprocessor.MaybeHandleDefine("", symbols)
        self.assertEqual(symbols, {})
        shader_preprocessor.MaybeHandleDefine("#define FOO", symbols)
        self.assertEqual(symbols, {"FOO": None})

    def test_parse_include(self):
        self.fs.create_file("/foo/bar",
                            contents=SIMPLE_SOURCE_WITH_INCLUDES_TEXT)
        self.fs.create_file("/foo/baz", contents="abc\n")
        self.fs.create_file("/foo/bar2", contents="xyz\n")
        source = shader_preprocessor.Source.Load("/foo/bar", {})
        self.assertEqual(
            source.Render(),
            textwrap.dedent("""
                            abc
                            xyz

                            varying float f;
                            """))

    def test_include_guards(self):
        filesystem = {
            "foo": """
                   #include "bar"
                   #include "baz"
                   """,
            "bar": """
                   #ifndef BAR
                   #define BAR
                   #include "baz"
                   #endif
                   """,
            "baz": """
                   #ifndef BAZ
                   #define BAZ
                   baz text
                   #endif
                   """
        }
        for filename, contents in filesystem.items():
            self.fs.create_file(filename,
                                contents=textwrap.dedent(contents).lstrip())

        source = shader_preprocessor.Source.Load("foo", {})
        self.assertEqual(
            source.Render(),
            textwrap.dedent("""
                            baz text
                            """).lstrip())


if __name__ == '__main__':
    unittest.main()
