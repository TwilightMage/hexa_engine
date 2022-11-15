#include "hexa_engine/Texture.h"

#include "base_lib/Assert.h"
#include "base_lib/File.h"
#include "base_lib/stb.h"
#include "hexa_engine/Game.h"

#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>

Texture::Texture()
{
    ogre_texture_ = Ogre::TextureManager::getSingletonPtr()->create("", "");
}

Texture::Texture(const Array2D<Color>& pixels)
    : Texture()
{
    put_pixels(pixels);
}

Texture::Texture(uint width, uint height, const List<Color>& pixels)
    : Texture(Array2D(width, height, pixels))
{
}

Shared<Texture> Texture::load_png(const Path& path)
{
    uint w, h;
    Array2D<Color> colors = stb::load(path, w, h);

    auto result = MakeShared<Texture>();
    result->put_pixels(colors);

    return result;

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
    return ogre_texture_->getWidth();
}

uint Texture::get_height() const
{
    return ogre_texture_->getHeight();
}

Vector2 Texture::get_size() const
{
    return Vector2(static_cast<float>(ogre_texture_->getWidth()), static_cast<float>(ogre_texture_->getHeight()));
}

Color Texture::get_pixel(uint x, uint y) const
{
    Color pixel;
    ogre_texture_->getBuffer()->readData((y * ogre_texture_->getWidth() + x) * sizeof(Color), sizeof(Color), &pixel);
    return pixel;
}

void Texture::save_to_file(const Path& path)
{
    Ogre::Image img;
    ogre_texture_->convertToImage(img);
    img.save(path.get_absolute_string().c());
}

void Texture::put_pixels(const Array2D<Color>& pixels)
{
    put_pixels(pixels.get_size_x(), pixels.get_size_y(), pixels.to_list());
}

void Texture::put_pixels(uint width, uint height, const List<Color>& pixels)
{
    ogre_texture_->loadImage(Ogre::Image(Ogre::PF_R8G8B8A8, width, height, 4, (byte*)pixels.get_data()));
}
