#include "hexa_engine/physics/Collision.h"

#include "hexa_engine/Game.h"

Shared<reactphysics3d::PhysicsCommon> Collision::get_physics()
{
    return Game::instance_->physics_;
}
