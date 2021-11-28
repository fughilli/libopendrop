import unittest
import parameterized
from libopendrop.preset import wrap_shader
from pyfakefs.fake_filesystem_unittest import patchfs

FILE_A = ("""#ifndef FILE_A
#define FILE_A
foo
bar
baz
#endif  // FILE_A
""")

EXPECTED_FILE_A = ("""foo
bar
baz
""")

FILE_B = ("""#ifndef FILE_B
#define FILE_B
buzz
qux
#endif  // FILE_B
""")

EXPECTED_FILE_B = ("""buzz
qux
""")

FILE_C = ("""#ifndef FILE_C
#define FILE_C
#include <file_a>
blah
#endif  // FILE_C
""")

EXPECTED_FILE_C = ("""foo
bar
baz
blah
""")

FILE_D = ("""#ifndef FILE_D
#define FILE_D
#include <file_c>
#include <file_b>
bruh
#endif
""")

EXPECTED_FILE_D = ("""foo
bar
baz
blah
buzz
qux
bruh
""")

FILE_E = ("""#ifndef FILE_E
#define FILE_E
#include <file_a>
#include <file_c>
quz
bux
#endif  // FILE_E
""")

EXPECTED_FILE_E = ("""foo
bar
baz
blah
quz
bux
""")


class TestWrapShader(unittest.TestCase):
    @parameterized.parameterized.expand([(FILE_A, EXPECTED_FILE_A),
                                         (FILE_B, EXPECTED_FILE_B)])
    def test_no_includes(self, file_text, expected_file_text):
        self.assertEqual(wrap_shader.ProcessMacros('file_a', file_text, []),
                         expected_file_text)

    @patchfs
    def test_single_include(self, fs):
        fs.create_file('file_a', contents=FILE_A)
        self.assertEqual(wrap_shader.ProcessMacros('file_c', FILE_C, []),
                         EXPECTED_FILE_C)

    @patchfs
    def test_nested_include(self, fs):
        fs.create_file('file_a', contents=FILE_A)
        fs.create_file('file_b', contents=FILE_B)
        fs.create_file('file_c', contents=FILE_C)
        self.assertEqual(wrap_shader.ProcessMacros('file_d', FILE_D, []),
                         EXPECTED_FILE_D)

    @patchfs
    def test_repeat_include(self, fs):
        fs.create_file('file_a', contents=FILE_A)
        fs.create_file('file_c', contents=FILE_C)
        self.assertEqual(wrap_shader.ProcessMacros('file_e', FILE_E, []),
                         EXPECTED_FILE_E)


if __name__ == '__main__':
    unittest.main()
