#include "hexa_engine/physics/ConcaveMeshCollision.h"

#include <reactphysics3d/collision/TriangleVertexArray.h>
#include <reactphysics3d/engine/PhysicsCommon.h>

ConcaveMeshCollision::ConcaveMeshCollision(const List<StaticMesh::Vertex>& vertices, const List<uint>& indices)
    : indices_copy_(indices)
{
    vertices_copy_ = List<Vector3>(vertices.length());
    for (uint i = 0; i < vertices.length(); i++)
    {
        vertices_copy_[i] = vertices[i].pos;
    }
    
    triangle_vertex_array = new reactphysics3d::TriangleVertexArray(
        vertices_copy_.length(),
        vertices_copy_.get_data(),
        sizeof(Vector3),
        indices_copy_.length() / 3,
        indices_copy_.get_data(),
        sizeof(int) * 3,
        reactphysics3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
        reactphysics3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE
    );
    
    triangle_mesh = get_physics()->createTriangleMesh();
    triangle_mesh->addSubpart(triangle_vertex_array);
    
    concave_mesh_shape = get_physics()->createConcaveMeshShape(triangle_mesh);
}

ConcaveMeshCollision::~ConcaveMeshCollision()
{
    get_physics()->destroyConcaveMeshShape(concave_mesh_shape);
    get_physics()->destroyTriangleMesh(triangle_mesh);
    delete triangle_vertex_array;
}

reactphysics3d::CollisionShape* ConcaveMeshCollision::get_collider_shape() const
{
    return concave_mesh_shape;
}
