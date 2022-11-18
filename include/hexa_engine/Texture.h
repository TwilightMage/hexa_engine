#pragma once

#include "ModuleAssetID.h"

#include <OgreHeaderSuffix.h>
#include <base_lib/Array2D.h>
#include <base_lib/Color.h>
#include <base_lib/List.h>
#include <base_lib/Path.h>
#include <base_lib/Pointers.h>
#include <map>

class Material;
class Module;
class Game;
class UIElement;
class Image;

namespace Ogre {
    class Texture;
}

class EXPORT Texture {
private:
    friend Module;
    friend Material;

public:
    uint get_width() const;
    uint get_height() const;
    Color get_pixel(uint x, uint y) const;

    void save_to_file(const Path& path);

    void put_pixels(const Array2D<Color>& pixels);
    Array2D<Color> get_pixels() const;

    const ModuleAssetID& get_id() const { return id_; }

private:
    Texture();

    Shared<Ogre::Texture> ogre_texture_;
    ModuleAssetID id_;
};
