#include "hexa_engine/StaticMesh.h"

#include "hexa_engine/Game.h"
#include "hexa_engine/GeometryEditor.h"
#include "hexa_engine/physics/BoxCollision.h"
#include "hexa_engine/physics/ConcaveMeshCollision.h"
#include "hexa_engine/physics/ConvexMeshCollision.h"
#include "hexa_engine/physics/SphereCollision.h"

#include <OBJ_Loader.h>
#include <OgreHardwareBufferManager.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreSubMesh.h>
#include <OgreVertexIndexData.h>
#include <base_lib/Assert.h>
#include <base_lib/File.h>
#include <base_lib/performance.h>

struct Bounds
{
    Vector3 min;
    Vector3 max;

    FORCEINLINE void add(const Vector3& point)
    {
        min.x = Math::min(min.x, point.x);
        min.y = Math::min(min.y, point.y);
        min.z = Math::min(min.z, point.z);
        max.x = Math::max(max.x, point.x);
        max.y = Math::max(max.y, point.y);
        max.z = Math::max(max.z, point.z);
    }

    FORCEINLINE Vector3 get_center() const { return (max + min) * 0.5f; };
    FORCEINLINE Vector3 get_extents() const { return (max - min) * 0.5f; };
};

Shared<StaticMesh> StaticMesh::empty = MakeShared<StaticMesh>("Empty Mesh");

StaticMesh::SubMesh::SubMesh(const String& name, const List<Vertex>& vertices, const List<uint>& indices)
    : name(name)
    , vertices(vertices)
    , indices(indices)
{
}

StaticMesh::SubMesh::SubMesh(const List<Vertex>& vertices, const List<uint>& indices)
    : SubMesh("", vertices, indices)
{
}

StaticMesh::SubMesh::SubMesh()
    : SubMesh("", {}, {})
{
}

void StaticMesh::SubMesh::add(const List<Vertex>& new_vertices, const List<uint>& new_indices)
{
    uint offset = vertices.length();
    vertices.add_many(new_vertices);
    for (auto& ind : new_indices)
    {
        indices.add(ind + offset);
    }
}

StaticMesh::StaticMesh(const String& name)
    : instanced_(false)
{
}

Shared<StaticMesh> StaticMesh::construct(const String& name, const List<SubMesh>& sub_meshes, AutoCollisionMode collision_mode, bool compute_normals)
{
    const auto result = create(name, sub_meshes, collision_mode, compute_normals);

    verbose("Mesh", "Constructed mesh %s", name.c());

    return result;
}

Shared<StaticMesh> StaticMesh::load_file_obj(const Path& path, AutoCollisionMode collision_mode)
{
    if (const auto found = Game::instance_->meshes_.find(path.get_absolute_string()))
    {
        return *found;
    }

    if (!Check(path.exists(), "Mesh Loader", "Mesh does not exists %s", path.get_absolute_string().c()))
        return nullptr;

    Shared<objl::Loader> loader = MakeShared<objl::Loader>();
    if (!Check(loader->LoadFile(path.get_absolute().to_string().std()), "Mesh Loader", "Unknown error on loading mesh %s", path.get_absolute_string().c()))
        return nullptr;
    if (!Check(loader->LoadedVertices.size() > 0, "Mesh Loader", "Number of vertices is 0 in mesh %s", path.get_absolute_string().c()))
        return nullptr;
    if (!Check(loader->LoadedIndices.size() > 0, "Mesh Loader", "Number of indices is 0 in mesh %s", path.get_absolute_string().c()))
        return nullptr;

    List<SubMesh> sub_meshes(loader->LoadedMeshes.size());
    for (uint i = 0; i < sub_meshes.length(); i++)
    {
        auto& sub_mesh = sub_meshes[i];
        const auto src_sub_mesh = loader->LoadedMeshes[i];
        sub_mesh.name = src_sub_mesh.MeshName;
        sub_mesh.vertices = List<Vertex>(src_sub_mesh.Vertices.size());
        for (uint j = 0; j < src_sub_mesh.Vertices.size(); j++)
        {
            const auto& vert = src_sub_mesh.Vertices[j];
            sub_mesh.vertices[j] = Vertex{
                cast_object<Vector3>(vert.Position) * 100,
                Vector2(vert.TextureCoordinate.X, 1 - vert.TextureCoordinate.Y),
                cast_object<Vector3>(vert.Normal)};
        }

        sub_mesh.indices = src_sub_mesh.Indices;

        GeometryEditor::optimize(sub_mesh.vertices, sub_mesh.indices);
        GeometryEditor::rotate(sub_mesh.vertices, Quaternion::from_axis_angle(Vector3::forward(), 90));
    }

    Shared<StaticMesh> result = create(path.filename + path.extension, sub_meshes, collision_mode, false);

    Game::instance_->meshes_[path.get_absolute_string()] = result;
    verbose("Mesh", "Loaded mesh %s", path.get_absolute_string().c());

    return result;
}

uint StaticMesh::get_material_count() const
{
    return ogre_mesh_->getNumSubMeshes();
}

const Vector3& StaticMesh::get_bounds_center() const
{
    return cast_object<Vector3>(ogre_mesh_->getBounds().getCenter());
}

const Vector3& StaticMesh::get_bounds_half_size() const
{
    return cast_object<Vector3>(ogre_mesh_->getBounds().getHalfSize());
}

bool StaticMesh::is_empty() const
{
    return ogre_mesh_->sharedVertexData->vertexCount == 0;
}

void StaticMesh::make_instanced()
{
    instanced_ = true;
}

struct TriangleNormal
{
    StaticMesh::Triangle triangle;
    Vector3 center;
    Vector3 normal;
};

Shared<StaticMesh> StaticMesh::create(const String& name, const List<SubMesh>& sub_meshes, AutoCollisionMode collision_mode, bool compute_normals)
{
    Shared<StaticMesh> result = MakeShared<StaticMesh>(name);

    result->ogre_mesh_ = Ogre::MeshManager::getSingleton().createManual(name.c(), Ogre::RGN_DEFAULT);

    result->ogre_mesh_->sharedVertexData = new Ogre::VertexData();

    Ogre::VertexDeclaration* decl = result->ogre_mesh_->sharedVertexData->vertexDeclaration;
    Ogre::VertexBufferBinding* bind = result->ogre_mesh_->sharedVertexData->vertexBufferBinding;

    size_t offset = 0;
    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

    uint visible_vertex_count = 0;
    uint visible_index_count = 0;
    for (const auto& sub_mesh : sub_meshes)
    {
        if (!sub_mesh.name.starts_with("SPHERE_") &&
            !sub_mesh.name.starts_with("BOX_") &&
            !sub_mesh.name.starts_with("CONVEX_"))
        {
            visible_vertex_count += sub_mesh.vertices.length();
            visible_index_count += sub_mesh.indices.length();
        }
    }

    List<Vertex> vertices_copy(visible_vertex_count);
    List<uint> indices_copy(visible_index_count);

    uint vci = 0;
    uint ici = 0;

    uint vertex_offset = 0;
    Bounds visual_bounds;
    for (const auto& sub_mesh : sub_meshes)
    {
        if (sub_mesh.name.starts_with("SPHERE_")) // Sphere collision
        {
            if (collision_mode == AutoCollisionMode::Default)
            {
                Bounds sub_bounds;
                for (const auto& vert : sub_mesh.vertices)
                {
                    sub_bounds.add(cast_object<Vector3>(vert.pos));
                }

                result->collisions_.add({sub_bounds.get_center(), Quaternion(), MakeShared<SphereCollision>(sub_bounds.get_extents().get_min_axis())});
            }
        }
        else if (sub_mesh.name.starts_with("BOX_")) // Box collision
        {
            if (collision_mode == AutoCollisionMode::Default)
            {
                if (sub_mesh.indices.length() == 3 * 2 * 6)
                {
                    List<TriangleNormal> triangles(2 * 6);
                    for (uint i = 0; i < 2 * 6; i++)
                    {
                        triangles[i] = {
                            sub_mesh.indices[i * 3 + 0],
                            sub_mesh.indices[i * 3 + 1],
                            sub_mesh.indices[i * 3 + 2]};
                        triangles[i].normal = GeometryEditor::compute_normal(
                            sub_mesh.vertices[triangles[i].triangle.i0].pos,
                            sub_mesh.vertices[triangles[i].triangle.i1].pos,
                            sub_mesh.vertices[triangles[i].triangle.i2].pos);
                        triangles[i].center = (sub_mesh.vertices[triangles[i].triangle.i0].pos + sub_mesh.vertices[triangles[i].triangle.i1].pos + sub_mesh.vertices[triangles[i].triangle.i2].pos) / 3.f;
                    }

                    List<Pair<uint, uint>> sides;
                    List<uint> triangle_indices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
                    bool error = false;
                    for (uint i = 0; i < 6; i++)
                    {
                        bool pair_found = false;
                        for (uint j = 1; j < triangle_indices.length(); j++)
                        {
                            if (1.f - triangles[triangle_indices[0]].normal.dot_product(triangles[triangle_indices[j]].normal) < KINDA_SMALL_NUMBER)
                            {
                                sides.add({triangle_indices[0], triangle_indices[j]});
                                triangle_indices.remove_at(j);
                                triangle_indices.remove_at(0);
                                pair_found = true;
                                break;
                            }
                        }

                        if (!pair_found)
                        {
                            error = true;
                            break;
                        }
                    }

                    if (error)
                    {
                        print_error("Static Mesh", "Failed to construct box collision from sub-mesh %s, proposed sub-mesh must be a box with 2 triangles on each side", sub_mesh.name.c());
                        continue;
                    }

                    List<Pair<uint, uint>> opposites;
                    List<uint> side_indices = {0, 1, 2, 3, 4, 5};
                    for (uint i = 0; i < 3; i++)
                    {
                        bool opposite_found = false;
                        for (uint j = 1; j < side_indices.length(); j++)
                        {
                            if (1.f + triangles[sides[side_indices[0]].key].normal.dot_product(triangles[sides[side_indices[j]].key].normal) < KINDA_SMALL_NUMBER)
                            {
                                opposites.add({side_indices[0], side_indices[j]});
                                side_indices.remove_at(j);
                                side_indices.remove_at(0);
                                opposite_found = true;
                                break;
                            }
                        }

                        if (!opposite_found)
                        {
                            error = true;
                            break;
                        }
                    }

                    if (error)
                    {
                        print_error("Static Mesh", "Failed to construct box collision from sub-mesh %s, proposed sub-mesh must be a box with 2 triangles on each side", sub_mesh.name.c());
                        continue;
                    }

                    Quaternion box_rotation = Quaternion::look_at(triangles[sides[opposites[0].key].key].normal, triangles[sides[opposites[1].key].key].normal);
                    Vector3 box_size = Vector3(
                        Vector3::distance((triangles[sides[opposites[0].key].key].center + triangles[sides[opposites[0].key].value].center) / 2,
                                          (triangles[sides[opposites[0].value].key].center + triangles[sides[opposites[0].value].value].center) / 2) /
                            2,
                        Vector3::distance((triangles[sides[opposites[1].key].key].center + triangles[sides[opposites[1].key].value].center) / 2,
                                          (triangles[sides[opposites[1].value].key].center + triangles[sides[opposites[1].value].value].center) / 2) /
                            2,
                        Vector3::distance((triangles[sides[opposites[2].key].key].center + triangles[sides[opposites[2].key].value].center) / 2,
                                          (triangles[sides[opposites[2].value].key].center + triangles[sides[opposites[2].value].value].center) / 2) /
                            2);
                    Vector3 box_center = (triangles[0].center + triangles[1].center + triangles[2].center + triangles[3].center + triangles[4].center + triangles[5].center + triangles[6].center + triangles[7].center + triangles[8].center + triangles[9].center + triangles[10].center + triangles[11].center) / 12.f;

                    result->collisions_.add({box_center, box_rotation, MakeShared<BoxCollision>(box_size)});
                }
                else
                {
                    print_error("Static Mesh", "Failed to construct box collision from sub-mesh %s, proposed sub-mesh must be a box with 2 triangles on each side", sub_mesh.name.c());
                }
            }
        }
        else if (sub_mesh.name.starts_with("CONVEX_")) // Convex collision
        {
            if (collision_mode == AutoCollisionMode::Default)
            {
                auto vertices = sub_mesh.vertices;

                Bounds sub_bounds;
                for (const auto& vert : vertices)
                {
                    sub_bounds.add(cast_object<Vector3>(vert.pos));
                }

                const Vector3 sub_mesh_center = sub_bounds.get_center();
                if (sub_mesh_center != Vector3::zero())
                {
                    for (auto& vert : vertices)
                    {
                        vert.pos -= sub_mesh_center;
                    }
                }

                result->collisions_.add({sub_mesh_center, Quaternion(), MakeShared<ConvexMeshCollision>(vertices, sub_mesh.indices)});
            }
        }
        else // Visible mesh
        {
            auto vertices = sub_mesh.vertices;

            if (compute_normals)
            {
                GeometryEditor::compute_normals(vertices, sub_mesh.indices, true);
            }

            memcpy(vertices_copy.get_data() + vci, vertices.get_data(), sizeof(Vertex) * vertices.length());
            vci += vertices.length();

            for (uint i = 0; i < vertices.length(); i++)
            {
                visual_bounds.add(vertices[i].pos);
            }

            Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(Ogre::HardwareIndexBuffer::IT_32BIT, sub_mesh.indices.length(), Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
            List<uint> indices_buff(sub_mesh.indices.get_data(), sub_mesh.indices.length());
            for (uint i = 0; i < sub_mesh.indices.length(); i++)
            {
                indices_buff[i] = sub_mesh.indices[i] + vertex_offset;
            }

            if (collision_mode == AutoCollisionMode::Complex || collision_mode == AutoCollisionMode::Convex)
            {
                memcpy(indices_copy.get_data() + ici, indices_buff.get_data(), sizeof(uint) * indices_buff.length());
                ici += indices_buff.length();
            }

            ibuf->writeData(0, sizeof(uint) * indices_buff.length(), indices_buff.get_data(), false);

            auto sub = result->ogre_mesh_->createSubMesh();
            sub->useSharedVertices = true;
            sub->indexData->indexBuffer = ibuf;
            sub->indexData->indexCount = sub_mesh.indices.length();
            sub->indexData->indexStart = 0;

            vertex_offset += vertices.length();
        }
    }

    result->ogre_mesh_->sharedVertexData->vertexCount = vertices_copy.length();

    Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(offset, vertices_copy.length(), Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    vbuf->writeData(0, sizeof(Vertex) * vertices_copy.length(), vertices_copy.get_data());
    bind->setBinding(0, vbuf);

    if (collision_mode == AutoCollisionMode::Complex)
    {
        result->collisions_.add({Vector3::zero(), Quaternion(), MakeShared<ConcaveMeshCollision>(vertices_copy, indices_copy)});
    }
    else if (collision_mode == AutoCollisionMode::Convex)
    {
        result->collisions_.add({Vector3::zero(), Quaternion(), MakeShared<ConvexMeshCollision>(vertices_copy, indices_copy)});
    }

    result->ogre_mesh_->_setBounds(Ogre::AxisAlignedBox(cast_object<Ogre::Vector3>(visual_bounds.min), cast_object<Ogre::Vector3>(visual_bounds.max)));
    // result->ogre_mesh_->buildEdgeList();

    return result;
}
