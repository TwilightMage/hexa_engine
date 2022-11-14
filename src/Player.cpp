#include "hexa_engine/Player.h"

#include "hexa_engine/CameraComponent.h"
#include "hexa_engine/Game.h"

Player::Player()
    : Entity()
{
    set_tick_enabled(true);

    camera_ = create_component<CameraComponent>();
}

void Player::on_possess()
{
    Game::use_camera(camera_);
}
