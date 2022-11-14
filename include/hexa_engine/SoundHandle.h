#pragma once

#include <base_lib/BasicTypes.h>
#include <base_lib/Pointers.h>
#include <base_lib/framework.h>

class World;

namespace SoLoud
{
    class Soloud;
}

class EXPORT SoundHandle
{
    friend World;

public:
    SoundHandle();

    void stop();

    void set_pause(bool pause) const;

    void set_volume(float vol) const;
    float get_volume() const;

private:
    SoundHandle(int handle);

    int handle_ = 0;
};
