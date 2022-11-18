#pragma once

#include <base_lib/Compound.h>
#include <base_lib/framework.h>

class EXPORT Settings
{
public:
    virtual ~Settings() {}

    float get_physics_tick_interval() const { return physics_tick_interval_; }

    uint fps_limit = 60;

    float audio_general = 1.0f;

    virtual void read_settings(const Compound::Object& compound);
    virtual Compound::Object write_settings();

private:
    float physics_tick_interval_ = 1.0f / 60.0f;
};
