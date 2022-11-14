#pragma once

#include <base_lib/BasicTypes.h>
#include <base_lib/Path.h>

class EXPORT ITexture
{
public:
    virtual uint get_gl_texture_id() = 0;

    virtual void save_to_file(const Path& path) = 0;
};
