#pragma once

#include "Collision.h"

#include <base_lib/Vector3.h>

namespace reactphysics3d
{
    class SphereShape;
}

class EXPORT SphereCollision : public Collision
{
public:
    SphereCollision(float radius);
    ~SphereCollision();

private:
    FORCEINLINE reactphysics3d::CollisionShape* get_collider_shape() const override;

    reactphysics3d::SphereShape* shape_;
};
