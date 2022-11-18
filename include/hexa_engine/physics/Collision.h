#pragma once

#include <base_lib/Pointers.h>
#include <base_lib/framework.h>

namespace reactphysics3d
{
    class CollisionShape;
    class PhysicsCommon;
    class Collider;
} // namespace reactphysics3d

class EXPORT Collision
{
public:
    virtual reactphysics3d::CollisionShape* get_collider_shape() const = 0;

protected:
    static Shared<reactphysics3d::PhysicsCommon> get_physics();
};
