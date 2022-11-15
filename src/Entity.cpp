#include "hexa_engine/Entity.h"

#include "hexa_engine/Game.h"
#include "hexa_engine/GeometryEditor.h"
#include "hexa_engine/physics/Collision.h"
#include "hexa_engine/StaticMesh.h"
#include "hexa_engine/World.h"
#include "base_lib/Assert.h"
#include "hexa_engine/CollisionMaskBits.h"

#include <OgreSceneNode.h>
#include <reactphysics3d/body/RigidBody.h>
#include <reactphysics3d/engine/PhysicsCommon.h>

Entity::Entity()
    : EnableSharedFromThis<Entity>()
    , started_(false)
{
}

Shared<World> Entity::get_world() const
{
    return world_.lock();
}

void Entity::on_start()
{
}

void Entity::on_tick(float delta_time)
{
}

void Entity::on_destroy()
{
}

void Entity::destroy()
{
    if (auto world = get_world())
    {
        world->mark_entity_for_destroy(shared_from_this(), true);
    }
}

bool Entity::is_started() const
{
    return started_;
}

void Entity::set_transform(const Transform& transform)
{
    set_location(transform.location);
    set_rotation(transform.rotation);
    set_scale(transform.scale);
}

void Entity::set_location(const Vector3& location)
{
    transform_.location = location;
    if (scene_node_)
    {
        scene_node_->setPosition(cast_object<Ogre::Vector3>(location));
    }
}

void Entity::translate(const Vector3& translation)
{
    set_location(get_location() + translation);
}

void Entity::set_rotation(const Quaternion& rot)
{
    transform_.rotation = rot;
    if (scene_node_)
    {
        scene_node_->setOrientation(Ogre::Quaternion(rot.w, rot.x, rot.y, rot.z));
    }
}

void Entity::rotate(const Vector3& axis, float angle)
{
    set_rotation(get_rotation().rotate_around(axis, angle));
}

void Entity::set_scale(const Vector3& scale)
{
    transform_.scale = scale;
    if (scene_node_)
    {
        scene_node_->setScale(cast_object<Ogre::Vector3>(scale));
    }
}

void Entity::set_collision(const Shared<Collision>& collision, const Vector3& offset)
{
    /*remove_collision();

    const auto collider_shape = collision->get_collider_shape();

    Assert(collider_shape != nullptr);

    if (!rigid_body_)
    {
        print_warning("Physics", "Attempt to set collider for entity which is not a rigid body!");
        return;
    }
    
    collision_ = collision;
    collider_ = rigid_body_->addCollider(collider_shape, reactphysics3d::Transform(cast_object<reactphysics3d::Vector3>(offset), reactphysics3d::Quaternion::identity()));
    collider_->setCollisionCategoryBits(collision_mask_);*/
}

void Entity::remove_collision()
{
    /*if (!rigid_body_)
    {
        print_warning("Physics", "Attempt to remove collider from entity which is not a rigid body!");
        return;
    }
    
    if (collider_)
    {
        rigid_body_->removeCollider(collider_);
        collider_ = nullptr;
    }

    collision_ = nullptr;*/
}

void Entity::set_collision_mask(byte16 bits)
{
    /*collision_mask_ = bits;
    
    if (collider_)
    {
        collider_->setCollisionCategoryBits(bits);
    }*/
}

byte16 Entity::get_collision_mask() const
{
    //return collision_mask_;
    return CollisionMaskBits::NONE;
}

void Entity::set_gravity_enabled(bool state) const
{
    /*if (!rigid_body_)
    {
        print_warning("Physics", "Attempt to set gravity for entity which is not a rigid body!");
        return;
    }
    
    rigid_body_->enableGravity(state);*/
}

void Entity::make_body_static()
{
    /*if (!rigid_body_)
    {
        print_warning("Physics", "Attempt to make entity static which is not a rigid body!");
        return;
    }

    update_physically_dynamic();
    
    rigid_body_->setType(reactphysics3d::BodyType::STATIC);*/
}

void Entity::make_body_dynamic()
{
    /*if (!rigid_body_)
    {
        print_warning("Physics", "Attempt to make entity dynamic which is not a rigid body!");
        return;
    }

    update_physically_dynamic();
    
    rigid_body_->setType(reactphysics3d::BodyType::DYNAMIC);*/
}

void Entity::make_body_kinematic()
{
    /*if (!rigid_body_)
    {
        print_warning("Physics", "Attempt to make entity kinematic which is not a rigid body!");
        return;
    }

    update_physically_dynamic();
    
    rigid_body_->setType(reactphysics3d::BodyType::KINEMATIC);*/
}

void Entity::remove_all_components()
{
    for (auto& component : components_)
    {
        component->on_destroy();
        component->owner_ = nullptr;
    }
    components_.clear();
}

void Entity::set_tick_enabled(bool state)
{
    if (tick_enabled_ == state) return;

    tick_enabled_ = state;
    
    if (auto world = get_world())
    {
        world->set_entity_tick_enabled(shared_from_this(), state);
    }
}

void Entity::start()
{
    started_ = true;

    for (auto& component : components_)
    {
        component->start();
    }
    
    on_start();
}

void Entity::tick(float delta_time)
{
    on_tick(delta_time);
}
