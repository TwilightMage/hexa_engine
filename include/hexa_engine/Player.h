#pragma once

#include "Entity.h"
#include "IControllable.h"

class EXPORT Player : public Entity, public IControllable
{
public:
    Player();

    void on_possess() override;

    const Shared<CameraComponent>& camera() const { return camera_; }

private:
    Shared<CameraComponent> camera_;
};
