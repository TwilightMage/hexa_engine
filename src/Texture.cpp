#include "hexa_engine/Texture.h"

#include "base_lib/Assert.h"
#include "base_lib/File.h"
#include "base_lib/stb.h"
#include "hexa_engine/Game.h"

#include <OGRE/OgreTextureManager.h>

Texture::Texture()
{
}

Texture::Texture(const Array2D<Color>& pixels)
    : pixels_(pixels)
{
}

Texture::Texture(uint width, uint height, const List<Color>& pixels)
    : Texture(Array2D(width, height, pixels))
{
}

Shared<Texture> Texture::load_png(const Path& path)
{
    assert(false);
    return nullptr;

    /*if (const auto found = Game::instance_->textures_.find(path.get_absolute_string()))
    {
        return *found;
    }

    if (!Check(path.exists(), "Texture Loader", "Texture does not exist %s", path.get_absolute_string().c())) return nullptr;

    uint tex_width, tex_height, tex_channels;
    const auto pixels = stb::load(path, tex_width, tex_height);
    if (pixels.length() > 0)
    {
        if (tex_width * tex_height > 0)
        {
            const uint size = tex_width * tex_height;
            auto result = MakeShared<Texture>(path.get_absolute_string());
            result->pixels_ = Array2D(tex_width, tex_height, pixels);

            Game::instance_->textures_[path.get_absolute_string()] = result;
            verbose("Texture", "Loaded texture %ix%i %s", tex_width, tex_height, path.get_absolute_string().c());

            return result;
        }
        else
        {
            print_error("Texture", "Texture size is invalid: %ix%i %s", tex_width, tex_height, path.get_absolute_string().c());
        }
    }
    else
    {
        print_error("Texture", "Unknown error on loading texture %s", path.get_absolute_string().c());
    }
    return nullptr;*/
}

uint Texture::get_width() const
{
    return pixels_.get_size_x();
}

uint Texture::get_height() const
{
    return pixels_.get_size_y();
}

Vector2 Texture::get_size() const
{
    return Vector2(static_cast<float>(pixels_.get_size_x()), static_cast<float>(pixels_.get_size_y()));
}

Color Texture::get_pixel(uint x, uint y) const
{
    return pixels_.at(x, y);
}

void Texture::save_to_file(const Path& path)
{
    stb::save_bmp(path, pixels_.get_size_x(), pixels_.get_size_y(), pixels_);
}

void Texture::put_pixels(const Array2D<Color>& pixels)
{
    put_pixels(pixels.get_size_x(), pixels.get_size_y(), pixels.to_list());
}

void Texture::put_pixels(uint width, uint height, const List<Color>& pixels)
{
    if (edit_count_ == 0 && pixels_.get_size_x() == width && pixels_.get_size_y() == height)
    {
        pixels_ = Array2D(width, height, pixels);

        if (usage_count() > 0)
        {
            unload();
            load_internal();
        }
    }
}
