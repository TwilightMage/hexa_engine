#include "hexa_engine/physics/ConvexMeshCollision.h"

#include <reactphysics3d/collision/PolygonVertexArray.h>
#include <reactphysics3d/engine/PhysicsCommon.h>

ConvexMeshCollision::ConvexMeshCollision(const List<StaticMesh::Vertex>& vertices, const List<uint>& indices)
    : indices_copy_(indices)
{
    auto source_vertices = vertices;
    auto source_indices = indices;
    
    GeometryEditor::optimize_collision(source_vertices, source_indices);
    
    GeometryEditor::compute_faces(source_vertices, source_indices, faces, indices_copy_);
    
    vertices_copy_ = List<Vector3>(source_vertices.length());
    for (uint i = 0; i < source_vertices.length(); i++)
    {
        vertices_copy_[i] = source_vertices[i].pos;
    }

    polygon_vertex_array = new reactphysics3d::PolygonVertexArray(
        vertices_copy_.length(),
        vertices_copy_.get_data(),
        sizeof(Vector3),
        indices_copy_.get_data(),
        sizeof(uint),
        faces.length(),
        reinterpret_cast<reactphysics3d::PolygonVertexArray::PolygonFace*>(faces.get_data()),
        reactphysics3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
        reactphysics3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE
    );
    
    polyhedron_mesh = get_physics()->createPolyhedronMesh(polygon_vertex_array);
    
    convex_mesh_shape = get_physics()->createConvexMeshShape(polyhedron_mesh);
}

ConvexMeshCollision::~ConvexMeshCollision()
{
    get_physics()->destroyConvexMeshShape(convex_mesh_shape);
    get_physics()->destroyPolyhedronMesh(polyhedron_mesh);
    delete polygon_vertex_array;
}

reactphysics3d::CollisionShape* ConvexMeshCollision::get_collider_shape() const
{
    return convex_mesh_shape;
}
