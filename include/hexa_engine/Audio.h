#pragma once

#include <base_lib/BasicTypes.h>
#include <base_lib/Path.h>
#include <base_lib/Pointers.h>

class Game;
class World;

namespace SoLoud
{
    class Wav;
}

class EXPORT Audio
{
    friend World;
    friend Game;

public:
    static Shared<Audio> load(const Path& path);

    void set_looped(bool looped);
    bool is_looped() const { return looped_; }

    void set_default_volume(float volume);
    float get_default_volume() const { return default_volume_; }

    float get_duration() const;

private:
    Shared<SoLoud::Wav> sample_;
    bool looped_ = false;
    float default_volume_ = 1.0f;
};
