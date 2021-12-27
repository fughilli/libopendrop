import hashlib
import re
import os
from typing import Text, Any, List, Tuple

from absl import app
from absl import flags

flags.DEFINE_string("object_filename", None, "Input OBJ-format model file")
flags.DEFINE_string(
    "outputs", None,
    "Output header and source filenames, separated by a space")

FLAGS = flags.FLAGS

HEADER_TEMPLATE = ("""#ifndef {guard_symbol}
#define {guard_symbol}

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace {namespace} {{

constexpr int __kVertexCount_{hash_string} = {vertex_count};
constexpr int __kNormalCount_{hash_string} = {normal_count};
constexpr int __kTriangleCount_{hash_string} = {triangle_count};
extern const std::array<glm::vec3, __kVertexCount_{hash_string}> __kVertices_{hash_string};
extern const std::array<glm::vec3, __kNormalCount_{hash_string}> __kNormals_{hash_string};
extern const std::array<glm::vec2, __kVertexCount_{hash_string}> __kUvs_{hash_string};
extern const std::array<glm::uvec3, __kTriangleCount_{hash_string}> __kTriangles_{hash_string};

static inline const std::array<glm::vec3, __kVertexCount_{hash_string}>& Vertices() {{
  return __kVertices_{hash_string};
}}

static inline const std::array<glm::vec3, __kNormalCount_{hash_string}>& Normals() {{
  return __kNormals_{hash_string};
}}

static inline const std::array<glm::vec2, __kVertexCount_{hash_string}>& Uvs() {{
  return __kUvs_{hash_string};
}}

static inline const std::array<glm::uvec3, __kTriangleCount_{hash_string}>& Triangles() {{
  return __kTriangles_{hash_string};
}}

}}

#endif // {guard_symbol}
""")

SOURCE_TEMPLATE = ("""#include "{header_filename}"

namespace {namespace} {{

const std::array<glm::vec3, __kVertexCount_{hash_string}> __kVertices_{hash_string} =
{vertices_initializer};
const std::array<glm::vec3, __kNormalCount_{hash_string}> __kNormals_{hash_string} =
{normals_initializer};
const std::array<glm::vec2, __kVertexCount_{hash_string}> __kUvs_{hash_string} =
{uvs_initializer};
const std::array<glm::uvec3, __kTriangleCount_{hash_string}> __kTriangles_{hash_string} =
{triangles_initializer};

}}
""")

Vertex = Tuple[float]
Normal = Tuple[float]
Uv = Tuple[float]

# A FaceIndex is a pair of vertex and uv indices. The indices point to a pair of
# vertex and uv coordinates from a model's vertex and uv lists, 1-indexed.
FaceIndex = Tuple[int]

# A Face is a set of FaceIndex that describe one face in a model.
Face = List[FaceIndex]

# A CollapsedFace is a set of indices that point to the uniformly-indexed vertex
# and UV data for a model.
CollapsedFace = List[int]

ModelData = Tuple[List[Vertex], List[Normal], List[Uv], List[Face]]
CollapsedModelData = Tuple[List[Vertex], List[Normal], List[Uv],
                           List[CollapsedFace]]


def MakeHashString(object_filename: Text, object_text: Text):
    return hashlib.sha1(
        (object_filename + object_text).encode('utf-8')).hexdigest()


def MakeGuardSymbol(object_filename: Text) -> Text:
    return '{}_H_'.format(
        object_filename.translate(str.maketrans('.-/', '___')).upper())


def MakeNamespace(object_filename: Text) -> Text:
    return os.path.basename(object_filename).translate(
        str.maketrans('.-', '__')).lower()


def GetHeaderFilename(outputs: Text) -> Text:
    return outputs.split(' ')[0]


def GetSourceFilename(outputs: Text) -> Text:
    return outputs.split(' ')[1]


def GenerateHeader(guard_symbol: Text, namespace: Text, vertex_count: int,
                   normal_count: int, triangle_count: int,
                   hash_string: Text) -> Text:
    return HEADER_TEMPLATE.format(guard_symbol=guard_symbol,
                                  namespace=namespace,
                                  vertex_count=str(vertex_count),
                                  normal_count=str(normal_count),
                                  triangle_count=str(triangle_count),
                                  hash_string=hash_string)


def GenerateSource(header_filename: Text, namespace: Text,
                   vertices_initializer: Text, normals_initializer: Text,
                   uvs_initializer: Text, triangles_initializer: Text,
                   hash_string: Text) -> Text:
    return SOURCE_TEMPLATE.format(header_filename=header_filename,
                                  namespace=namespace,
                                  vertices_initializer=vertices_initializer,
                                  normals_initializer=normals_initializer,
                                  uvs_initializer=uvs_initializer,
                                  triangles_initializer=triangles_initializer,
                                  hash_string=hash_string)


def FormatInitializer(initializer: Any) -> Text:
    def FormatInitializerList(initializer_list: List[Any]) -> Text:
        return "{{{{{}}}}}".format(",".join(
            FormatInitializer(element) for element in initializer_list))

    def FormatInitializerTuple(initializer_list: Tuple[Any]) -> Text:
        return "{{{}}}".format(",".join(
            FormatInitializer(element) for element in initializer_list))

    return {
        str: (lambda: "\"{}\"".format(initializer)),
        int: (lambda: str(initializer)),
        float: (lambda: "{}f".format(initializer)),
        list: (lambda: FormatInitializerList(initializer)),
        tuple: (lambda: FormatInitializerTuple(initializer)),
    }[type(initializer)]()


def TriangulatePolygon(indices: List[Any]) -> List[List[Any]]:
    common_vertex = indices[0]
    return_triangles = []
    for i in range(1, len(indices) - 1):
        return_triangles.append([common_vertex] + indices[i:i + 2])
    return return_triangles


def MakeCapturingFloatRegex(capture_name: Text,
                            include_negative: bool) -> Text:
    return ("(?P<" + capture_name + ">" +
            ("[-]?" if include_negative else "") + "[0-9]+(\.[0-9]*)?)")


def MakeCapturingIntegerRegex(capture_name: Text,
                              include_negative: bool) -> Text:
    return ("(?P<" + capture_name + ">" +
            ("[-]?" if include_negative else "") + "[0-9]+)")


def ParseObjectVertexText(object_text: Text) -> List[Vertex]:
    VERTEX_RE = re.compile(r"^v\s+{}\s+{}\s+{}$".format(
        MakeCapturingFloatRegex('x', True), MakeCapturingFloatRegex('y', True),
        MakeCapturingFloatRegex('z', True)))

    return_vertex_list = []

    for line in object_text.split(os.linesep):
        match = VERTEX_RE.match(line)
        if match is None:
            continue

        vertex_components = tuple(
            float(match.groupdict()[component]) for component in 'xyz')
        return_vertex_list.append(vertex_components)

    return return_vertex_list


def ParseObjectNormalText(object_text: Text) -> List[Normal]:
    NORMAL_RE = re.compile(r"^vn\s+{}\s+{}\s+{}$".format(
        MakeCapturingFloatRegex('x', True), MakeCapturingFloatRegex('y', True),
        MakeCapturingFloatRegex('z', True)))

    return_normal_list = []

    for line in object_text.split(os.linesep):
        match = NORMAL_RE.match(line)
        if match is None:
            continue

        normal_components = tuple(
            float(match.groupdict()[component]) for component in 'xyz')
        return_normal_list.append(normal_components)

    return return_normal_list


def ParseObjectUvText(object_text: Text) -> List[Uv]:
    VERTEX_RE = re.compile(r"^vt\s+{}\s+{}$".format(
        MakeCapturingFloatRegex('u', True), MakeCapturingFloatRegex('v',
                                                                    True)))

    return_vertex_list = []

    for line in object_text.split(os.linesep):
        match = VERTEX_RE.match(line)
        if match is None:
            continue

        vertex_components = tuple(
            float(match.groupdict()[component]) for component in 'uv')
        return_vertex_list.append(vertex_components)

    return return_vertex_list


def ParseObjectFaceText(object_text: Text) -> List[Face]:
    INDEX_RE_STRING = (r"^{}/{}?(/{}?)?$".format(
        MakeCapturingIntegerRegex('v', False),
        MakeCapturingIntegerRegex('t', False),
        MakeCapturingIntegerRegex('n', False)))
    INDEX_RE = re.compile(INDEX_RE_STRING)

    face_list = []

    for line in object_text.split(os.linesep):
        line_parts = line.split()
        if len(line_parts) < 4:
            continue

        if line_parts[0] != 'f':
            continue

        face_indices = []
        for vertex_indices_string in line_parts[1:]:
            match = INDEX_RE.match(vertex_indices_string)

            vertex_indices = tuple(
                int(match.groupdict()[component]) for component in 'vnt')
            face_indices.append(vertex_indices)

        face_list.append(face_indices)

    return face_list


def ParseObjectText(object_text: Text) -> ModelData:
    result = (ParseObjectVertexText(object_text),
              ParseObjectNormalText(object_text),
              ParseObjectUvText(object_text), ParseObjectFaceText(object_text))
    return result


def CollapseIndices(vertex_list: List[Vertex], normal_list: List[Normal],
                    uv_list: List[Uv],
                    face_list: List[Face]) -> CollapsedModelData:
    collapsed_mapping = {}

    for face in face_list:
        for indices in face:
            if indices in collapsed_mapping:
                continue
            collapsed_mapping[indices] = len(collapsed_mapping) + 1

    collapsed_vertex_list = [None] * len(collapsed_mapping)
    collapsed_normal_list = [None] * len(collapsed_mapping)
    collapsed_uv_list = [None] * len(collapsed_mapping)

    for (vertex_index, normal_index,
         uv_index), collapsed_index in collapsed_mapping.items():
        vertex = vertex_list[vertex_index - 1]
        collapsed_vertex_list[collapsed_index - 1] = vertex
        normal = normal_list[normal_index - 1]
        collapsed_normal_list[collapsed_index - 1] = normal
        uv = uv_list[uv_index - 1]
        collapsed_uv_list[collapsed_index - 1] = uv

    collapsed_face_list = []
    for face in face_list:
        collapsed_face = []
        for indices in face:
            collapsed_face.append(collapsed_mapping[indices])
        collapsed_face_list.append(collapsed_face)

    return (collapsed_vertex_list, collapsed_normal_list, collapsed_uv_list,
            collapsed_face_list)


def LoadCollapsedModel(object_text: Text) -> CollapsedModelData:
    vertex_list, normal_list, uv_list, face_list = ParseObjectText(object_text)

    triangulated_face_list = []
    for face in face_list:
        triangulated_face_list.extend(TriangulatePolygon(face))

    return CollapseIndices(vertex_list, normal_list, uv_list,
                           triangulated_face_list)


def MakeIndicesZeroIndexed(
        face_list: List[CollapsedFace]) -> List[CollapsedFace]:
    zero_indexed_face_list = []

    for face in face_list:
        zero_indexed_face = []

        for index in face:
            zero_indexed_face.append(index - 1)

        zero_indexed_face_list.append(zero_indexed_face)

    return zero_indexed_face_list


def main(argv):
    if FLAGS.object_filename == None:
        raise ValueError("--object_filename must be specified")

    print("Absolute path:", os.path.abspath(os.curdir))
    print("Input:", FLAGS.object_filename)

    object_filename = FLAGS.object_filename

    object_text = open(object_filename, 'r').read()
    guard_symbol = MakeGuardSymbol(object_filename)
    namespace = MakeNamespace(object_filename)
    hash_string = MakeHashString(object_filename, object_text)
    outputs = FLAGS.outputs

    (collapsed_vertices, collapsed_normals, collapsed_uvs,
     collapsed_faces) = (LoadCollapsedModel(object_text))

    # Make the indices zero-indexed for loading into the GPU.
    collapsed_faces = MakeIndicesZeroIndexed(collapsed_faces)

    # Hack to make the inner initializers single-braced.
    collapsed_faces = [tuple(x) for x in collapsed_faces]

    (vertices_initializer, normals_initializer, uvs_initializer,
     triangles_initializer) = (FormatInitializer(model_list)
                               for model_list in (collapsed_vertices,
                                                  collapsed_normals,
                                                  collapsed_uvs,
                                                  collapsed_faces))

    header_filename = GetHeaderFilename(outputs)
    with open(header_filename, 'w') as header_file:
        header_file.write(
            GenerateHeader(guard_symbol=guard_symbol,
                           namespace=namespace,
                           vertex_count=len(collapsed_vertices),
                           normal_count=len(collapsed_normals),
                           triangle_count=len(collapsed_faces),
                           hash_string=hash_string))

    source_filename = GetSourceFilename(outputs)
    with open(source_filename, 'w') as source_file:
        source_file.write(
            GenerateSource(header_filename=header_filename,
                           namespace=namespace,
                           vertices_initializer=vertices_initializer,
                           normals_initializer=normals_initializer,
                           uvs_initializer=uvs_initializer,
                           triangles_initializer=triangles_initializer,
                           hash_string=hash_string))

    print("Wrote to", source_filename, "and", header_filename)


if __name__ == "__main__":
    app.run(main)
