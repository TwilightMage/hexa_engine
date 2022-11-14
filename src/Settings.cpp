#include "hexa_engine/Settings.h"

#include "base_lib/Math.h"

void Settings::read_settings(const Compound::Object& compound)
{
    const auto graphics = compound.get_object("graphics");
    fps_limit = Math::clamp(graphics.get_int32("fps cap", 60), 1, 10000);

    const auto audio = compound.get_object("audio");
    audio_general = Math::clamp(audio.get_float("general", 1.0f), 0.0f, 1.0f);
}

Compound::Object Settings::write_settings()
{
    return {
        {"graphics", Compound::Object{
            {"fps_cap", (int32)fps_limit}
        }},
        {"audio", Compound::Object{
            {"general", audio_general}
        }}
    };
}
