#pragma once

#include "CollisionMaskBits.h"
#include "EntityComponent.h"

#include <base_lib/Matrix4x4.h>
#include <base_lib/Name.h>
#include <base_lib/Quaternion.h>
#include <base_lib/Vector2.h>

class Material;

namespace Ogre
{
    class InstanceManager;
    class InstancedEntity;
    class Entity;
} // namespace Ogre

class Collision;

namespace reactphysics3d
{
    class Collider;
    class RigidBody;
} // namespace reactphysics3d

class StaticMesh;

enum class PhysicalBodyType
{
    Static,
    Kinematic,
    Dynamic
};

class EXPORT MeshComponent : public EntityComponent
{
public:
    explicit MeshComponent(const Shared<StaticMesh>& mesh, const List<Shared<Material>>& materials);
    explicit MeshComponent(const Shared<StaticMesh>& mesh, const Shared<Material>& material);
    explicit MeshComponent(const Shared<StaticMesh>& mesh);
    MeshComponent();

    void on_start() override;
    void on_destroy() override;

    Shared<StaticMesh> get_mesh() const { return mesh_; }
    void set_mesh(const Shared<StaticMesh>& mesh);
    void set_mesh(const Shared<StaticMesh>& mesh, const Shared<Material>& material);
    void set_mesh(const Shared<StaticMesh>& mesh, const List<Shared<Material>>& materials);

    uint get_material_count() const;
    void set_material(const Shared<Material>& material, uint material_slot);
    Shared<Material> get_material(uint material_slot) const;

    void set_material_parameter(Quaternion value, uint material_slot, uint parameter_index);

    PhysicalBodyType get_body_type() const { return body_type_; }
    void set_body_type(PhysicalBodyType body_type);

    bool is_visible() const { return is_visible_; }
    void set_visibility(bool state);

private:
    void spawn_mesh(const Shared<Entity>& owner, const Shared<World>& world);
    void update_visibility();
    void destroy_mesh(const Shared<Entity>& owner, const Shared<World>& world);
    Shared<Material> get_valid_material(uint slot);

    Shared<StaticMesh> mesh_;
    List<Shared<Material>> materials_;
    reactphysics3d::RigidBody* rigid_body_ = nullptr;
    reactphysics3d::Collider* collider_ = nullptr;
    Shared<Collision> collision_;
    byte16 collision_mask_ = CollisionMaskBits::NONE;
    PhysicalBodyType body_type_ = PhysicalBodyType::Dynamic;
    bool is_visible_ = true;

    Ogre::Entity* ogre_entity_ = nullptr;
    List<Ogre::InstancedEntity*> ogre_instanced_entities_;
    List<Ogre::InstanceManager*> cached_instance_managers_;
};
