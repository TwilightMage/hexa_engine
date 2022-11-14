#include "hexa_engine/physics/SphereCollision.h"

#include <reactphysics3d/engine/PhysicsCommon.h>

SphereCollision::SphereCollision(float radius)
{
    shape_ = get_physics()->createSphereShape(radius);
}

SphereCollision::~SphereCollision()
{
    get_physics()->destroySphereShape(shape_);
}

reactphysics3d::CollisionShape* SphereCollision::get_collider_shape() const
{
    return shape_;
}
