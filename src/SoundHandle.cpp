#include "hexa_engine/SoundHandle.h"

#include "hexa_engine/Game.h"

#include <soloud/soloud.h>

SoundHandle::SoundHandle()
    : handle_(0)
{
}

void SoundHandle::stop()
{
    Game::instance_->soloud_->stop(handle_);
    handle_ = 0;
}

void SoundHandle::set_pause(bool pause) const
{
    Game::instance_->soloud_->setPause(handle_, pause);
}

void SoundHandle::set_volume(float vol) const
{
    Game::instance_->soloud_->setVolume(handle_, vol);
}

float SoundHandle::get_volume() const
{
    return Game::instance_->soloud_->getVolume(handle_);
}

SoundHandle::SoundHandle(int handle)
    : handle_(handle)
{
}
