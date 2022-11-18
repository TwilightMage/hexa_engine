#include "hexa_engine/MeshComponent.h"

#include "hexa_engine/Entity.h"
#include "hexa_engine/Game.h"
#include "hexa_engine/Material.h"
#include "hexa_engine/StaticMesh.h"
#include "hexa_engine/World.h"
#include "hexa_engine/physics/Collision.h"

#include <OgreEntity.h>
#include <OgreInstancedEntity.h>
#include <OgreMaterialManager.h>
#include <OgreMesh.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreSubEntity.h>
#include <reactphysics3d/body/RigidBody.h>
#include <reactphysics3d/collision/Collider.h>
#include <reactphysics3d/engine/PhysicsWorld.h>

MeshComponent::MeshComponent(const Shared<StaticMesh>& mesh, const List<Shared<Material>>& materials)
    : mesh_(mesh)
    , materials_(materials)
{
    materials_.resize(mesh->ogre_mesh_->getNumSubMeshes(), nullptr);
}

MeshComponent::MeshComponent(const Shared<StaticMesh>& mesh, const Shared<Material>& material)
    : MeshComponent(mesh, List<Shared<Material>>(mesh->ogre_mesh_->getNumSubMeshes(), material))
{
}

MeshComponent::MeshComponent(const Shared<StaticMesh>& mesh)
    : MeshComponent(mesh, nullptr)
{
}

MeshComponent::MeshComponent()
    : mesh_(nullptr)
    , materials_(List<Shared<Material>>())
{
}

void MeshComponent::on_start()
{
    if (auto owner = get_owner())
    {
        if (auto world = owner->get_world())
        {
            rigid_body_ = world->physics_world_->createRigidBody(reactphysics3d::Transform(cast_object<reactphysics3d::Vector3>(owner->get_location()), cast_object<reactphysics3d::Quaternion>(owner->get_rotation())));
            rigid_body_->setUserData(owner.get());
            rigid_body_->setType((reactphysics3d::BodyType)body_type_);

            if (mesh_)
            {
                spawn_mesh(owner, world);
            }
        }
    }
}

void MeshComponent::on_destroy()
{
    if (const auto owner = get_owner())
    {
        if (const auto world = owner->get_world())
        {
            if (mesh_)
            {
                destroy_mesh(owner, world);
            }

            world->physics_world_->destroyRigidBody(rigid_body_);
        }
    }
}

void MeshComponent::set_mesh(const Shared<StaticMesh>& mesh)
{
    set_mesh(mesh, nullptr);
}

void MeshComponent::set_mesh(const Shared<StaticMesh>& mesh, const Shared<Material>& material)
{
    set_mesh(mesh, List<Shared<Material>>(mesh ? mesh->ogre_mesh_->getNumSubMeshes() : 0, material));
}

void MeshComponent::set_mesh(const Shared<StaticMesh>& mesh, const List<Shared<Material>>& materials)
{
    if (mesh_ == mesh)
    {
        if (mesh == nullptr)
            return;

        auto temp_materials = materials;
        temp_materials.resize(mesh->ogre_mesh_->getNumSubMeshes(), nullptr);

        if (materials_ == temp_materials)
            return;

        for (uint i = 0; i < temp_materials.length(); i++)
        {
            set_material(temp_materials[i], i);
        }
        return;
    }

    if (const auto owner = get_owner())
    {
        if (const auto world = owner->get_world())
        {
            if (mesh_)
            {
                destroy_mesh(owner, world);
            }

            mesh_ = mesh;
            materials_ = materials;
            materials_.resize(mesh->ogre_mesh_->getNumSubMeshes(), nullptr);

            if (mesh_)
            {
                spawn_mesh(owner, world);
            }
        }
    }
}

uint MeshComponent::get_material_count() const
{
    return materials_.length();
}

void MeshComponent::set_material(const Shared<Material>& material, uint material_slot)
{
    if (mesh_ && material_slot < mesh_->ogre_mesh_->getNumSubMeshes())
    {
        if (materials_[material_slot] == material)
            return;

        materials_[material_slot] = material;

        if (mesh_->instanced_)
        {
            auto manager = ogre_instanced_entities_[material_slot]->_getManager();
            auto owner = ogre_instanced_entities_[material_slot]->getParentSceneNode();

            owner->detachObject(ogre_instanced_entities_[material_slot]);
            manager->destroyInstancedEntity(ogre_instanced_entities_[material_slot]);

            ogre_instanced_entities_[material_slot] = cached_instance_managers_[material_slot]->createInstancedEntity(get_valid_material(material_slot)->ogre_material_);
            owner->attachObject(ogre_instanced_entities_[material_slot]);

            if (material_slot == 0)
            {
                for (uint i = 1; i < mesh_->ogre_mesh_->getNumSubMeshes(); i++)
                {
                    ogre_instanced_entities_[0]->shareTransformWith(ogre_instanced_entities_[i]);
                }
            }
            else
            {
                ogre_instanced_entities_[0]->shareTransformWith(ogre_instanced_entities_[material_slot]);
            }
        }
        else
        {
            ogre_entity_->getSubEntity(material_slot)->setMaterial(get_valid_material(material_slot)->ogre_material_);
        }
    }
}

Shared<Material> MeshComponent::get_material(uint material_slot) const
{
    if (mesh_ && material_slot < mesh_->ogre_mesh_->getNumSubMeshes())
    {
        return materials_[material_slot];
    }

    return nullptr;
}

void MeshComponent::set_material_parameter(Quaternion value, uint material_slot, uint parameter_index)
{
    return;
    if (mesh_ && material_slot < mesh_->ogre_mesh_->getNumSubMeshes())
    {
        if (mesh_->instanced_)
        {
            ogre_instanced_entities_[material_slot]->setCustomParam(parameter_index, cast_object<Ogre::Vector4>(value));
        }
        else
        {
            ogre_entity_->getSubEntity(material_slot)->setCustomParameter(parameter_index, cast_object<Ogre::Vector4>(value));
        }
    }
}

void MeshComponent::set_body_type(PhysicalBodyType body_type)
{
    if (body_type_ == body_type)
        return;

    body_type_ = body_type;

    if (rigid_body_)
    {
        rigid_body_->setType((reactphysics3d::BodyType)body_type);
    }
}

void MeshComponent::set_visibility(bool state)
{
    if (is_visible_ == state)
        return;

    is_visible_ = state;

    if (mesh_)
    {
        update_visibility();
    }
}

void MeshComponent::spawn_mesh(const Shared<Entity>& owner, const Shared<World>& world)
{
    if (mesh_->instanced_)
    {
        cached_instance_managers_ = world->get_or_create_instance_managers(mesh_, 10000, 0);

        for (uint i = 0; i < mesh_->ogre_mesh_->getNumSubMeshes(); i++)
        {
            auto ent = cached_instance_managers_[i]->createInstancedEntity(get_valid_material(i)->ogre_material_);
            if (i > 0)
                ogre_instanced_entities_[0]->shareTransformWith(ent);
            owner->scene_node_->attachObject(ent);
            ogre_instanced_entities_.add(ent);
        }
    }
    else
    {
        ogre_entity_ = world->manager_->createEntity(mesh_->name_.c());

        for (uint i = 0; i < materials_.length(); i++)
        {
            ogre_entity_->getSubEntity(i)->setMaterial(get_valid_material(i)->ogre_material_);
        }

        owner->scene_node_->attachObject(ogre_entity_);
    }

    if (!is_visible_)
    {
        update_visibility();
    }

    for (auto& collision : mesh_->collisions_)
    {
        rigid_body_->addCollider(collision.collision->get_collider_shape(), reactphysics3d::Transform(cast_object<reactphysics3d::Vector3>(collision.location), cast_object<reactphysics3d::Quaternion>(collision.rotation)));
    }
}

void MeshComponent::update_visibility()
{
    if (mesh_->instanced_)
    {
        for (uint i = 0; i < mesh_->ogre_mesh_->getNumSubMeshes(); i++)
        {
            ogre_instanced_entities_[i]->setInUse(is_visible_);
        }
    }
    else
    {
        for (uint i = 0; i < ogre_entity_->getNumSubEntities(); i++)
        {
            ogre_entity_->getSubEntity(i)->setVisible(is_visible_);
        }
    }
}

void MeshComponent::destroy_mesh(const Shared<Entity>& owner, const Shared<World>& world)
{
    if (mesh_->instanced_)
    {
        for (auto& instanced_entity : ogre_instanced_entities_)
        {
            owner->scene_node_->detachObject(instanced_entity);
            world->manager_->destroyInstancedEntity(instanced_entity);
        }

        ogre_instanced_entities_.clear();
        cached_instance_managers_.clear();
    }
    else
    {
        owner->scene_node_->detachObject(ogre_entity_);
        world->manager_->destroyEntity(ogre_entity_);
    }

    while (rigid_body_->getNbColliders())
    {
        rigid_body_->removeCollider(rigid_body_->getCollider(0));
    }
}

Shared<Material> MeshComponent::get_valid_material(uint slot)
{
    return materials_[slot] ? materials_[slot] : Game::get_basic_material();
}
