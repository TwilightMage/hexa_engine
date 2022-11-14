#pragma once

#include "StaticMesh.h"

#include <base_lib/Color.h>
#include <base_lib/List.h>
#include <base_lib/Quaternion.h>
#include <base_lib/framework.h>

class EXPORT GeometryEditor
{
public:
    struct Face
    {
        uint size;
        uint index;
    };

    FORCEINLINE static Vector3 compute_normal(const Vector3& a, const Vector3& b, const Vector3& c);

    // merge vertices, same by pos, uv and normal
    static void optimize(List<StaticMesh::Vertex>& vertices, List<uint>& indices);
    // merge vertices, same by pos
    static void optimize_collision(List<StaticMesh::Vertex>& vertices, List<uint>& indices);
    // remove indices for vertex rendering
    static void remove_indices(List<StaticMesh::Vertex>& vertices, List<uint>& indices);
    // invert triangles in vertex-only geometry
    static void invert_vertices(List<StaticMesh::Vertex>& vertices);
    // invert triangles in vertex-index geometry
    static void invert_indices(List<uint>& indices);
    // mirror along X axis
    static void mirror_x(List<StaticMesh::Vertex>& vertices);
    // mirror along Y axis
    static void mirror_y(List<StaticMesh::Vertex>& vertices);
    // mirror along Z axis
    static void mirror_z(List<StaticMesh::Vertex>& vertices);
    // translate geometry
    static void translate(List<StaticMesh::Vertex>& vertices, const Vector3& offset);
    // rotate geometry
    static void rotate(List<StaticMesh::Vertex>& vertices, const Quaternion& quat);
    // scale geometry
    static void scale(List<StaticMesh::Vertex>& vertices, const Vector3& factor);
    // move geometry to center
    static void move_to_center(List<StaticMesh::Vertex>& vertices);
    // group triangles by same normals
    static void compute_faces(const List<StaticMesh::Vertex>& vertices, const List<uint>& indices, List<Face>& out_faces, List<uint>& out_indices);
    // calculate normals from triangles into separate array
    static void compute_normals(const List<StaticMesh::Vertex>& vertices, const List<uint>& indices, List<Vector3>& out_normals, bool invert = false);
    // compute normals for triangles in place
    static void compute_normals(List<StaticMesh::Vertex>& vertices, const List<uint>& indices, bool invert = false);
    // remove vertices that are not referred by indices
    static void remove_unused_vertices(List<StaticMesh::Vertex>& vertices, List<uint>& indices);

    static Shared<StaticMesh> get_unit_cube();
};
