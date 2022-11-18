#include "hexa_engine/Texture.h"

#include <base_lib/Assert.h>
#include <base_lib/File.h>
#include <base_lib/stb.h>
#include "hexa_engine/Game.h"

#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>

uint Texture::get_width() const {
    return ogre_texture_->getWidth();
}

uint Texture::get_height() const {
    return ogre_texture_->getHeight();
}

Color Texture::get_pixel(uint x, uint y) const {
    Color pixel;
    ogre_texture_->getBuffer()->readData((y * ogre_texture_->getWidth() + x) * sizeof(Color), sizeof(Color), &pixel);
    return pixel;
}

void Texture::save_to_file(const Path& path) {
    Ogre::Image img;
    ogre_texture_->convertToImage(img);
    img.save(path.get_absolute_string().c());
}

void Texture::put_pixels(const Array2D<Color>& pixels) {
    ogre_texture_->loadImage(Ogre::Image(Ogre::PF_R8G8B8A8, pixels.get_size_x(), pixels.get_size_y(), 4, (byte*)pixels.begin(), false));
}

Array2D<Color> Texture::get_pixels() const {
    Array2D<Color> pixels(get_width(), get_height());
    ogre_texture_->getBuffer()->readData(0, pixels.size() * sizeof(Color), pixels.begin());
    return pixels;
}

Texture::Texture() {
}
