import unittest
import parameterized
from libopendrop.tools import wrap_model

OBJECT_MODEL_TEXT = (r"""
v 1 2 3
v 4 5 6
v 7 8 9
v 10 11 12
vt 1 2
vt 3 4
vt 5 6
vt 7 8
vt 9 1
vt 2 3
vn 1 2 3
vn 2 3 4
vn 3 4 5
f 1/1/1 2/2/2 3/3/3
f 2/4/1 1/5/2 4/6/3
f 3/1/1 2/2/2 4/3/3
f 1/4/1 3/5/2 4/6/3
""")


class TestWrapModel(unittest.TestCase):
    @parameterized.parameterized.expand([
        ('single_integer', 1, '1'),
        ('single_float', 2.3, '2.3f'),
        ('single_string', 'abc', '"abc"'),
        ('tuple', (1, 2, 3), '{1,2,3}'),
        ('list', [1, 2, 3], '{{1,2,3}}'),
        ('nested_list', [1, 2, [3, 4, 5]], '{{1,2,{{3,4,5}}}}'),
        ('nested_tuple', (1, 2, (3, 4, 5)), '{1,2,{3,4,5}}'),
        ('nested_tuple_in_list', [1, 2, (3, 4, 5)], '{{1,2,{3,4,5}}}'),
        ('string_list', ['abc', 'xyz'], '{{"abc","xyz"}}'),
    ])
    def test_format_initializer(self, name, format_input, expected):
        self.assertEqual(wrap_model.FormatInitializer(format_input), expected)

    @parameterized.parameterized.expand([
        ('triangle_noop', [1, 2, 3], [[1, 2, 3]]),
        ('quad_two_triangles', [1, 2, 3, 4], [[1, 2, 3], [1, 3, 4]]),
        (
            'pentagon_three_triangles',
            [1, 2, 3, 4, 5],  # Input
            [[1, 2, 3], [1, 3, 4], [1, 4, 5]]  # Expected
        ),
        (
            'triangle_noop_with_uvs',
            [(1, 1), (2, 2), (3, 3)],  # Input
            [[(1, 1), (2, 2), (3, 3)]]  # Expected
        ),
        (
            'quad_two_triangles_with_uvs',
            [(1, 1), (2, 2), (3, 3), (4, 4)],  # Input
            [[(1, 1), (2, 2), (3, 3)], [(1, 1), (3, 3), (4, 4)]]  # Expected
        ),
        (
            'pentagon_three_triangles_with_uvs',
            [(1, 1), (2, 2), (3, 3), (4, 4), (5, 5)],  # Input
            [[(1, 1), (2, 2), (3, 3)], [(1, 1), (3, 3), (4, 4)],
             [(1, 1), (4, 4), (5, 5)]]  # Expected
        ),
    ])
    def test_triangulate_polygon(self, name, input_polygon,
                                 expected_triangles):
        self.assertEqual(wrap_model.TriangulatePolygon(input_polygon),
                         expected_triangles)

    @parameterized.parameterized.expand([
        ('empty', '', []),
        (
            'single_normal',
            'vn 1.0 2.0 3.0',  # Input
            [(1.0, 2.0, 3.0)]  # Expected
        ),
        (
            'multiple_normals',
            'vn 1.0 2.0 3.0\nvn -2 3.14 15.777',  # Input
            [(1.0, 2.0, 3.0), (-2, 3.14, 15.777)]  # Expected
        ),
        (
            'garbage_but_empty',
            'the quick brown\nfox jumped over\nthe lazy dog\n',  # Input
            []  # Expected
        )
    ])
    def test_parse_object_normal_text(self, name, input_object_text, expected):
        self.assertEqual(wrap_model.ParseObjectNormalText(input_object_text),
                         expected)

    @parameterized.parameterized.expand([
        ('empty', '', []),
        (
            'single_vertex',
            'v 1.0 2.0 3.0',  # Input
            [(1.0, 2.0, 3.0)]  # Expected
        ),
        (
            'multiple_vertices',
            'v 1.0 2.0 3.0\nv -2 3.14 15.777',  # Input
            [(1.0, 2.0, 3.0), (-2, 3.14, 15.777)]  # Expected
        ),
        (
            'garbage_but_empty',
            'the quick brown\nfox jumped over\nthe lazy dog\n',  # Input
            []  # Expected
        )
    ])
    def test_parse_object_vertex_text(self, name, input_object_text, expected):
        self.assertEqual(wrap_model.ParseObjectVertexText(input_object_text),
                         expected)

    @parameterized.parameterized.expand([
        ('empty', '', []),
        (
            'single_uv',
            'vt 0.123 0.456',  # Input
            [(0.123, 0.456)]  # Expected
        ),
        (
            'multiple_uvs',
            'vt 1.0 2.0\nvt 2 -3.14',  # Input
            [(1.0, 2.0), (2, -3.14)]  # Expected
        ),
        (
            'garbage_but_empty',
            'the quick brown\nfox jumped over\nthe lazy dog\n',  # Input
            []  # Expected
        )
    ])
    def test_parse_object_uv_text(self, name, input_object_text, expected):
        self.assertEqual(wrap_model.ParseObjectUvText(input_object_text),
                         expected)

    @parameterized.parameterized.expand([
        ('empty', '', []),
        (
            'single_face',
            'f 1/2/3 4/5/6 7/8/9 10/11/12',  # Input
            [[(1, 3, 2), (4, 6, 5), (7, 9, 8), (10, 12, 11)]]  # Expected
        ),
        (
            'single_face_omit_normal',
            'f 1/2/ 4/5/ 7/8/ 10/11/',  # Input
            [[(1, None, 2), (4, None, 5), (7, None, 8),
              (10, None, 11)]]  # Expected
        ),
        (
            'single_face_omit_normal_and_slash',
            'f 1/2 4/5 7/8 10/11',  # Input
            [[(1, None, 2), (4, None, 5), (7, None, 8),
              (10, None, 11)]]  # Expected
        ),
        (
            'multiple_faces',
            'f 1/2/ 3/4/ 5/6/\nf 7/8/ 9/1/ 2/3/',  # Input
            [[(1, None, 2), (3, None, 4), (5, None, 6)],
             [(7, None, 8), (9, None, 1), (2, None, 3)]]  # Expected
        ),
        (
            'garbage_but_empty',
            'the quick brown\nfox jumped over\nthe lazy dog\n',  # Input
            []  # Expected
        )
    ])
    def test_parse_object_face_text(self, name, input_object_text, expected):
        self.assertEqual(wrap_model.ParseObjectFaceText(input_object_text),
                         expected)

    @parameterized.parameterized.expand([
        ('empty', ([], [], [], []), ([], [], [], [])),
        (
            'single_vertex_and_uv',
            (
                [(1.0, 2.0, 3.0)],  # Vertices
                [(4.0, 5.0, 6.0)],  # Normals
                [(7.0, 8.0)],  # UVs
                [[(1, 1, 1), (1, 1, 1), (1, 1, 1)]]  # Faces
            ),
            (
                [(1.0, 2.0, 3.0)],  # Vertices
                [(4.0, 5.0, 6.0)],  # Normals
                [(7.0, 8.0)],  # UVs
                [[1, 1, 1]]  # Faces
            )),
    ])
    def test_collapsed_indices_parameterized(self, name, input_tuple,
                                             expected_tuple):
        self.assertEqual(wrap_model.CollapseIndices(*input_tuple),
                         expected_tuple)

    @parameterized.parameterized.expand([
        ('noscale', [(0.0, 0.0, 0.0), (1.0, 1.0, 1.0)], [(0.0, 0.0, 0.0),
                                                         (1.0, 1.0, 1.0)]),
    ])
    def test_collapsed_indices_parameterized(self, name, input_vertex_list,
                                             expected_vertex_list):
        self.assertEqual(wrap_model.NormalizeVertices(input_vertex_list),
                         expected_vertex_list)

    def test_collapse_indices(self):
        # UV-textured tetrahedron.
        # Four points.
        vertex_list = [(1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12)]
        # Four normals.
        normal_list = [(2, 3, 4), (5, 6, 7), (8, 9, 10), (11, 12, 13)]
        # 6 UVs.
        uv_list = [(1, 2), (3, 4), (5, 6), (7, 8), (9, 1), (2, 3)]
        # Four triangles, textured alternately by the first and last 3 UVs in
        # the UV list.
        face_list = [[(1, 1, 1), (2, 2, 2), (3, 3, 3)],
                     [(2, 2, 4), (1, 1, 5), (4, 4, 6)],
                     [(3, 3, 1), (2, 2, 2), (4, 4, 3)],
                     [(1, 1, 4), (3, 3, 5), (4, 4, 6)]]

        # Unused; for documentation purposes.
        unique_vertices = [(1, 1), (2, 2), (3, 3), (2, 4), (1, 5), (4, 6),
                           (3, 1), (4, 3), (1, 4), (3, 5)]

        expected_vertex_list = [(1, 2, 3), (4, 5, 6), (7, 8, 9), (4, 5, 6),
                                (1, 2, 3), (10, 11, 12), (7, 8, 9),
                                (10, 11, 12), (1, 2, 3), (7, 8, 9)]
        expected_normal_list = [(2, 3, 4), (5, 6, 7), (8, 9, 10), (5, 6, 7),
                                (2, 3, 4), (11, 12, 13), (8, 9, 10),
                                (11, 12, 13), (2, 3, 4), (8, 9, 10)]
        expected_uv_list = [(1, 2), (3, 4), (5, 6), (7, 8), (9, 1), (2, 3),
                            (1, 2), (5, 6), (7, 8), (9, 1)]
        expected_face_list = [[1, 2, 3], [4, 5, 6], [7, 2, 8], [9, 10, 6]]
        self.assertEqual(
            wrap_model.CollapseIndices(vertex_list, normal_list, uv_list,
                                       face_list),
            (expected_vertex_list, expected_normal_list, expected_uv_list,
             expected_face_list))

    def test_parse_object_text(self):
        vertex_list = [(1.0, 2.0, 3.0), (4.0, 5.0, 6.0), (7.0, 8.0, 9.0),
                       (10.0, 11.0, 12.0)]
        normal_list = [(1.0, 2.0, 3.0), (2.0, 3.0, 4.0), (3.0, 4.0, 5.0)]
        uv_list = [(1.0, 2.0), (3.0, 4.0), (5.0, 6.0), (7.0, 8.0), (9.0, 1.0),
                   (2.0, 3.0)]
        face_list = [[(1, 1, 1), (2, 2, 2), (3, 3, 3)],
                     [(2, 1, 4), (1, 2, 5), (4, 3, 6)],
                     [(3, 1, 1), (2, 2, 2), (4, 3, 3)],
                     [(1, 1, 4), (3, 2, 5), (4, 3, 6)]]

        self.assertEqual(wrap_model.ParseObjectText(OBJECT_MODEL_TEXT),
                         (vertex_list, normal_list, uv_list, face_list))

    def test_load_collapsed_model(self):
        expected_vertex_list = [(1.0, 2.0, 3.0), (4.0, 5.0, 6.0),
                                (7.0, 8.0, 9.0), (4.0, 5.0, 6.0),
                                (1.0, 2.0, 3.0), (10.0, 11.0, 12.0),
                                (7.0, 8.0, 9.0), (10.0, 11.0, 12.0),
                                (1.0, 2.0, 3.0), (7.0, 8.0, 9.0)]
        expected_normal_list = [(1.0, 2.0, 3.0), (2.0, 3.0, 4.0),
                                (3.0, 4.0, 5.0), (1.0, 2.0, 3.0),
                                (2.0, 3.0, 4.0), (3.0, 4.0, 5.0),
                                (1.0, 2.0, 3.0), (3.0, 4.0, 5.0),
                                (1.0, 2.0, 3.0), (2.0, 3.0, 4.0)]
        expected_uv_list = [(1.0, 2.0), (3.0, 4.0), (5.0, 6.0), (7.0, 8.0),
                            (9.0, 1.0), (2.0, 3.0), (1.0, 2.0), (5.0, 6.0),
                            (7.0, 8.0), (9.0, 1.0)]
        expected_face_list = [[1, 2, 3], [4, 5, 6], [7, 2, 8], [9, 10, 6]]

        self.maxDiff = None
        self.assertEqual(wrap_model.LoadCollapsedModel(OBJECT_MODEL_TEXT),
                         (expected_vertex_list, expected_normal_list,
                          expected_uv_list, expected_face_list))


if __name__ == '__main__':
    unittest.main()
