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

namespace Ogre
{
    class Texture;
}

class EXPORT Texture
{
private:
    friend Game;
    friend Module;
    friend Material;

public:
    Texture();
    explicit Texture(const Array2D<Color>& pixels);
    Texture(uint width, uint height, const List<Color>& pixels);

    static Shared<Texture> load_png(const Path& path);

    uint get_width() const;
    uint get_height() const;
    Vector2 get_size() const;
    Color get_pixel(uint x, uint y) const;

    void save_to_file(const Path& path);

    void put_pixels(const Array2D<Color>& pixels);
    const Array2D<Color>& get_pixels() const { return pixels_; }
    void put_pixels(uint width, uint height, const List<Color>& pixels);

    const ModuleAssetID& get_id() const { return id_; }

private:
    Shared<Ogre::Texture> ogre_texture_;
    ModuleAssetID id_;
};
