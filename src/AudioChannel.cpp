#include "hexa_engine/AudioChannel.h"

#include "hexa_engine/Game.h"

#include <soloud/soloud_bus.h>

Shared<AudioChannel> AudioChannel::create()
{
    auto result = MakeSharedInternal(AudioChannel);
    result->bus_ = MakeShared<SoLoud::Bus>();
    result->bus_handle_ = Game::instance_->soloud_->play(*result->bus_);
    Game::instance_->audio_channels_.add(result);
    return result;
}

Shared<AudioChannel> AudioChannel::create(const Shared<AudioChannel>& parent)
{
    auto result = MakeSharedInternal(AudioChannel);
    result->bus_ = MakeShared<SoLoud::Bus>();
    result->bus_handle_ = parent->bus_->play(*result->bus_);
    Game::instance_->audio_channels_.add(result);
    return result;
}

void AudioChannel::set_volume(float volume)
{
    volume_ = volume;
    Game::instance_->soloud_->setVolume(bus_handle_, volume);
}

float AudioChannel::get_volume() const
{
    return volume_;
}

AudioChannel::AudioChannel()
{
}
