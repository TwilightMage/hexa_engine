#pragma once

#include "ModuleAssetID.h"

#include <base_lib/Pointers.h>

class Game;
class Texture;
class TileInfo;
class MeshComponent;
class Module;

namespace Ogre
{
    class Material;
}

class EXPORT Material
{
    friend Game;
    friend MeshComponent;

public:
    FORCEINLINE const ModuleAssetID& get_id() const { return id_; }

    uint get_textures_count() const;
    Shared<Texture> get_texture(uint index) const;
    void set_texture(const Shared<Texture>& texture, uint index);

    FORCEINLINE bool is_instanced() const { return ogre_material_instancing_ != nullptr; }

private:
    Shared<Ogre::Material> ogre_material_;
    Shared<Ogre::Material> ogre_material_instancing_;
    List<Shared<Texture>> textures_;
    ModuleAssetID id_;
};
