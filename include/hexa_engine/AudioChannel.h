#pragma once

#include <base_lib/BasicTypes.h>
#include <base_lib/Pointers.h>
#include <base_lib/framework.h>

class Game;
class World;

namespace SoLoud
{
    class Bus;
}

class EXPORT AudioChannel
{
    friend World;
    friend Game;

public:
    static Shared<AudioChannel> create();
    static Shared<AudioChannel> create(const Shared<AudioChannel>& parent);

    void set_volume(float volume);
    float get_volume() const;

private:
    AudioChannel();

    Shared<SoLoud::Bus> bus_;
    int bus_handle_ = 0;
    float volume_ = 1;
};
