import unittest
import parameterized
from libopendrop.preset import wrap_shader
from pyfakefs.fake_filesystem_unittest import patchfs

FILE_A = ("""foo
bar
baz
""")

FILE_B = ("""buzz
qux
""")

FILE_C = ("""#include <file_a>
blah
""")

EXPECTED_FILE_C = ("""foo
bar
baz
blah
""")

FILE_D = ("""#include <file_c>
#include <file_b>
bruh
""")

EXPECTED_FILE_D = ("""foo
bar
baz
blah
buzz
qux
bruh
""")


class TestWrapShader(unittest.TestCase):
    @parameterized.parameterized.expand([FILE_A, FILE_B])
    def test_no_includes(self, file_text):
        self.assertEqual(wrap_shader.ProcessIncludes(file_text, []), file_text)

    @patchfs
    def test_single_include(self, fs):
        fs.create_file('file_a', contents=FILE_A)
        self.assertEqual(wrap_shader.ProcessIncludes(FILE_C, []),
                         EXPECTED_FILE_C)

    @patchfs
    def test_nested_include(self, fs):
        fs.create_file('file_a', contents=FILE_A)
        fs.create_file('file_b', contents=FILE_B)
        fs.create_file('file_c', contents=FILE_C)
        self.assertEqual(wrap_shader.ProcessIncludes(FILE_D, []),
                         EXPECTED_FILE_D)


if __name__ == '__main__':
    unittest.main()
