#include "hexa_engine/Material.h"

#include "hexa_engine/Game.h"
#include "hexa_engine/Module.h"
#include "hexa_engine/Texture.h"

#include <OgreMaterial.h>
#include <OgreTechnique.h>

uint Material::get_textures_count() const
{
    return ogre_material_->getTechnique(0)->getPass(0)->getNumTextureUnitStates();
}

Shared<Texture> Material::get_texture(uint index) const
{
    if (index >= textures_.length()) return nullptr;

    return textures_[index];
}

void Material::set_texture(const Shared<Texture>& texture, uint index)
{
    if (index >= textures_.length()) return;

    textures_[index] = texture;

    ogre_material_->getTechnique(0)->getPass(0)->getTextureUnitState(index)->setTexture(texture ? Game::get_uv_test_texture()->ogre_texture_ : texture->ogre_texture_);
}
