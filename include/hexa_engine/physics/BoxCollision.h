#pragma once

#include "Collision.h"

#include <base_lib/Vector3.h>

namespace reactphysics3d
{
    class BoxShape;
}

class EXPORT BoxCollision : public Collision
{
public:
    BoxCollision(const Vector3& extent);
    ~BoxCollision();

private:
    reactphysics3d::CollisionShape* get_collider_shape() const override;

    reactphysics3d::BoxShape* shape_;
};
