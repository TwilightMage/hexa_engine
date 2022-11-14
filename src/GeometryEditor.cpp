#include "hexa_engine/GeometryEditor.h"

#include "base_lib/Math.h"

#include <map>

Vector3 GeometryEditor::compute_normal(const Vector3& a, const Vector3& b, const Vector3& c)
{
    return (c - a).cross_product(b - a).normalized();
}

void GeometryEditor::optimize(List<StaticMesh::Vertex>& vertices, List<uint>& indices)
{
    for (uint i = 0; i < vertices.length() - 1; i++)
    {
        for (uint j = i + 1; j < vertices.length(); j++)
        {
            if (vertices[i].pos == vertices[j].pos && vertices[i].uv == vertices[j].uv && vertices[i].norm == vertices[j].norm)
            {
                for (uint k = 0; k < indices.length(); k++)
                {
                    if (indices[k] == j)
                    {
                        indices[k] = i;
                    }
                    else if (indices[k] > j)
                    {
                        indices[k]--;
                    }
                }

                vertices.remove_at(j--);
            }
        }
    }
}

void GeometryEditor::optimize_collision(List<StaticMesh::Vertex>& vertices, List<uint>& indices)
{
    for (uint i = 0; i < vertices.length() - 1; i++)
    {
        for (uint j = i + 1; j < vertices.length(); j++)
        {
            if (vertices[i].pos == vertices[j].pos)
            {
                for (uint k = 0; k < indices.length(); k++)
                {
                    if (indices[k] == j)
                    {
                        indices[k] = i;
                    }
                    else if (indices[k] > j)
                    {
                        indices[k]--;
                    }
                }

                vertices.remove_at(j--);
            }
        }
    }
}

void GeometryEditor::remove_indices(List<StaticMesh::Vertex>& vertices, List<uint>& indices)
{
    /*for (uint i = 0; i < indices.length() - 1; i++)
    {
        if (indices[i] != i)
        {
            vertices.Insert(vertices[indices[i]], i);
            indices[i] = i;
            for (uint j = i + 1; j < indices.length(); j++)
            {
                if (indices[j] >= i) indices[j]++;
            }
        }
    }*/

    List<StaticMesh::Vertex> result;
    for (auto index : indices)
    {
        result.add(vertices[index]);
    }

    vertices = result;
    indices = List<uint>::generate(indices.length(), [](uint i) -> uint { return i; });
}

void GeometryEditor::invert_vertices(List<StaticMesh::Vertex>& vertices)
{
    for (uint i = 0; i < vertices.length(); i += 3)
    {
        std::swap(vertices[i], vertices[i + 2]);
        vertices[i + 0].norm *= -1;
        vertices[i + 1].norm *= -1;
        vertices[i + 2].norm *= -1;
    }
}

void GeometryEditor::invert_indices(List<uint>& indices)
{
    for (uint i = 0; i < indices.length(); i += 3)
    {
        std::swap(indices[i], indices[i + 2]);
    }
}

void GeometryEditor::mirror_x(List<StaticMesh::Vertex>& vertices)
{
    for (auto& vertex : vertices)
    {
        vertex.pos.x *= -1;
        vertex.norm.x *= -1;
    }
}

void GeometryEditor::mirror_y(List<StaticMesh::Vertex>& vertices)
{
    for (auto& vertex : vertices)
    {
        vertex.pos.y *= -1;
        vertex.norm.y *= -1;
    }
}

void GeometryEditor::mirror_z(List<StaticMesh::Vertex>& vertices)
{
    for (auto& vertex : vertices)
    {
        vertex.pos.z *= -1;
        vertex.norm.z *= -1;
    }
}

void GeometryEditor::translate(List<StaticMesh::Vertex>& vertices, const Vector3& offset)
{
    for (auto& vertex : vertices)
    {
        vertex.pos += offset;
    }
}

void GeometryEditor::rotate(List<StaticMesh::Vertex>& vertices, const Quaternion& quat)
{
    for (auto& vertex : vertices)
    {
        vertex.pos = quat.rotate_vector3(vertex.pos);
        vertex.norm = quat.rotate_vector3(vertex.norm);
    }
}

void GeometryEditor::scale(List<StaticMesh::Vertex>& vertices, const Vector3& factor)
{
    for (auto& vertex : vertices)
    {
        vertex.pos *= factor;
    }
}

void GeometryEditor::move_to_center(List<StaticMesh::Vertex>& vertices)
{
    Vector3 sum;
    for (const auto& vertex : vertices)
    {
        sum += vertex.pos;
    }
    sum /= static_cast<float>(vertices.length());
    translate(vertices, sum);
}

struct Triangle
{
    uint points[3];
    Vector3 normal;

    bool is_near(const Triangle& triangle)
    {
        byte counter = 0;
        for (byte i = 0; i < 3; i++)
        {
            for (byte j = 0; j < 3; j++)
            {
                if (points[i] == triangle.points[j])
                {
                    ++counter;
                    break;
                }
            }
        }
        return counter > 1;
    }
};

FORCEINLINE float angle(const Vector3& point, const Vector3& front, const Vector3& left)
{
    float angle = Math::acos_deg(Math::clamp(point.dot_product(front), -1.0f, 1.0f));
    float dot = Math::clamp(point.dot_product(left), -1.0f, 1.0f);
    if (dot > 0)
        angle *= -1;
    return Math::unwind_angle(angle);
}

void GeometryEditor::compute_faces(const List<StaticMesh::Vertex>& vertices, const List<uint>& indices, List<Face>& out_faces, List<uint>& out_indices)
{
    out_faces.clear();
    out_indices.clear();

    List<List<uint>> faces;
    List<Triangle> triangles(indices.length() / 3);

    // calculate normal for each triangle
    for (uint i = 0; i < indices.length() / 3; i++)
    {
        triangles[i].points[0] = indices[i * 3 + 0];
        triangles[i].points[1] = indices[i * 3 + 1];
        triangles[i].points[2] = indices[i * 3 + 2];
        triangles[i].normal = compute_normal(vertices[indices[i * 3 + 0]].pos, vertices[indices[i * 3 + 1]].pos, vertices[indices[i * 3 + 2]].pos);
    }

    // group triangles by normals
    for (uint i = 0; i < triangles.length(); i++)
    {
        bool face_found = false;
        for (auto& face : faces)
        {
            for (auto face_triangle_index : face)
            {
                if (triangles[i].normal == triangles[face_triangle_index].normal && triangles[i].is_near(triangles[face_triangle_index]))
                {
                    face_found = true;
                    face.add(i);
                    break;
                }
            }

            if (face_found)
                break;
        }

        if (!face_found)
        {
            faces.add({i});
        }
    }

    for (auto& face : faces)
    {
        // gather indices for face
        List<uint> face_vertex_indices;
        for (auto triangle_id : face)
        {
            face_vertex_indices.add_unique(triangles[triangle_id].points[0]);
            face_vertex_indices.add_unique(triangles[triangle_id].points[1]);
            face_vertex_indices.add_unique(triangles[triangle_id].points[2]);
        }

        // calculate face center
        Vector3 center;
        for (auto face_index : face_vertex_indices)
        {
            center += vertices[face_index].pos;
        }
        center /= face_vertex_indices.length();
        // order indices in counter-clockwise order
        Vector3 orient_axis_f = (vertices[face_vertex_indices[0]].pos - center).normalized();
        Vector3 orient_axis_l = orient_axis_f.cross_product(triangles[face[0]].normal);
        std::map<uint, float> angles;
        for (uint i = 0; i < face_vertex_indices.length(); i++)
        {
            angles[face_vertex_indices[i]] = angle((vertices[face_vertex_indices[i]].pos - center).normalized(), orient_axis_f, orient_axis_l);
        }
        face_vertex_indices.sort([&](uint a, uint b) -> bool {
            return angles[a] < angles[b];
        });

        out_faces.add({face_vertex_indices.length(), out_indices.length()});
        out_indices.add_many(face_vertex_indices);
    }
}

void GeometryEditor::compute_normals(const List<StaticMesh::Vertex>& vertices, const List<uint>& indices, List<Vector3>& out_normals, bool invert)
{
    out_normals.clear();

    for (uint i = 0; i < indices.length() / 3; i++)
    {
        auto normal = compute_normal(vertices[indices[i * 3 + 0]].pos, vertices[indices[i * 3 + 1]].pos, vertices[indices[i * 3 + 2]].pos);
        if (invert)
            normal *= -1;
        out_normals.add(normal);
    }
}

void GeometryEditor::compute_normals(List<StaticMesh::Vertex>& vertices, const List<uint>& indices, bool invert)
{
    List<List<Vector3>> vertex_normals = List<List<Vector3>>(vertices.length());

    for (uint i = 0; i < indices.length() / 3; i++)
    {
        const uint i0 = indices[i * 3 + 0];
        const uint i1 = indices[i * 3 + 1];
        const uint i2 = indices[i * 3 + 2];
        auto normal = compute_normal(vertices[i0].pos, vertices[i1].pos, vertices[i2].pos);
        if (invert)
            normal *= -1;
        vertex_normals[i0].add(normal);
        vertex_normals[i1].add(normal);
        vertex_normals[i2].add(normal);
    }

    for (uint i = 0; i < indices.length(); i++)
    {
        const uint index = indices[i];

        Vector3 normal_sum;
        for (auto norm : vertex_normals[index])
        {
            normal_sum += norm;
        }
        vertices[index].norm = (normal_sum / vertex_normals.length()).normalized();
    }
}

void GeometryEditor::remove_unused_vertices(List<StaticMesh::Vertex>& vertices, List<uint>& indices)
{
    List<bool> referred = List<bool>::generate(vertices.length(), false);

    for (auto index : indices)
    {
        referred[index] = true;
    }

    for (uint i = 0; i < vertices.length(); i++)
    {
        if (!referred[i])
        {
            vertices.remove_at(i);
            referred.remove_at(i);

            for (uint j = 0; j < indices.length(); j++)
            {
                if (indices[j] > i)
                {
                    indices[j--]--;
                }
            }
        }
    }
}

Shared<StaticMesh> GeometryEditor::get_unit_cube()
{
    static Shared<StaticMesh> result;
    if (result == nullptr)
    {
        List<StaticMesh::Vertex> vertices = {
            // down
            {{0, 0, 0}, {0, 1}, -Vector3::up()},
            {{1, 0, 0}, {1, 1}, -Vector3::up()},
            {{1, 0, 1}, {1, 0}, -Vector3::up()},
            {{0, 0, 1}, {0, 0}, -Vector3::up()},

            // left
            {{0, 0, 0}, {0, 1}, -Vector3::right()},
            {{0, 0, 1}, {1, 1}, -Vector3::right()},
            {{0, 1, 1}, {1, 0}, -Vector3::right()},
            {{0, 1, 0}, {0, 0}, -Vector3::right()},

            // back
            {{0, 0, 1}, {0, 1}, -Vector3::forward()},
            {{1, 0, 1}, {1, 1}, -Vector3::forward()},
            {{1, 1, 1}, {1, 0}, -Vector3::forward()},
            {{0, 1, 1}, {0, 0}, -Vector3::forward()},

            // up
            {{0, 1, 1}, {0, 1}, Vector3::up()},
            {{1, 1, 1}, {1, 1}, Vector3::up()},
            {{1, 1, 0}, {1, 0}, Vector3::up()},
            {{0, 1, 0}, {0, 0}, Vector3::up()},

            // right
            {{1, 0, 1}, {0, 1}, Vector3::right()},
            {{1, 0, 0}, {1, 1}, Vector3::right()},
            {{1, 1, 0}, {1, 0}, Vector3::right()},
            {{1, 1, 1}, {0, 0}, Vector3::right()},

            // front
            {{1, 0, 0}, {0, 1}, Vector3::forward()},
            {{0, 0, 0}, {1, 1}, Vector3::forward()},
            {{0, 1, 0}, {1, 0}, Vector3::forward()},
            {{1, 1, 0}, {0, 0}, Vector3::forward()},
        };

        List<uint> indices = {
            0,
            1,
            2,
            0,
            2,
            3,
            4,
            5,
            6,
            4,
            6,
            7,
            8,
            9,
            10,
            8,
            10,
            11,
            12,
            13,
            14,
            12,
            14,
            15,
            16,
            17,
            18,
            16,
            18,
            19,
            20,
            21,
            22,
            20,
            22,
            23,
        };

        translate(vertices, Vector3::one() * -0.5f);
        scale(vertices, Vector3(100));

        result = StaticMesh::construct("Unit Cube", {StaticMesh::SubMesh(vertices, indices)});
    }

    return result;
}
