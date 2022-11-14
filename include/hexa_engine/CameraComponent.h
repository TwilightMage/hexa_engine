#pragma once

#include "EntityComponent.h"

#include <base_lib/Vector2.h>

class Game;

namespace Ogre
{
    class SceneNode;
    class Camera;
} // namespace Ogre

class EXPORT CameraComponent : public EntityComponent
{
    friend Game;

public:
    void on_start() override;
    void on_destroy() override;

    bool WorldToViewport(const Vector3& world, Vector2& out_screen) const;
    Vector3 ViewportToWorld(const Vector2& screen) const;

private:
    Ogre::Camera* ogre_camera_ = nullptr;
};
