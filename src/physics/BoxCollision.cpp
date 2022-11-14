#include "hexa_engine/physics/BoxCollision.h"

#include <reactphysics3d/collision/shapes/BoxShape.h>
#include <reactphysics3d/engine/PhysicsCommon.h>

BoxCollision::BoxCollision(const Vector3& extent)
{
    shape_ = get_physics()->createBoxShape(cast_object<reactphysics3d::Vector3>(extent));
}

BoxCollision::~BoxCollision()
{
    get_physics()->destroyBoxShape(shape_);
}

reactphysics3d::CollisionShape* BoxCollision::get_collider_shape() const
{
    return shape_;
}
