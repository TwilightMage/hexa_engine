#pragma once

#include "ModuleAssetID.h"

#include <base_lib/Array2D.h>
#include <base_lib/Color.h>
#include <base_lib/Map.h>
#include <base_lib/Name.h>
#include <base_lib/Path.h>
#include <base_lib/Pointers.h>
#include <base_lib/Set.h>

class Shader;
class Texture;
class Material;
class Game;

namespace Ogre {
    class GpuProgram;
}

class EXPORT Module
{
    friend Game;

public:
    explicit Module(const Name& module_name);

    virtual void on_add_resource_directories(Set<String>& local, Set<String>& global);

    const Name& get_module_name() const
    {
        return module_name;
    }
    const ModuleAssetID get_asset_id(const Name& asset_name) const
    {
        return ModuleAssetID(get_module_name(), asset_name);
    }

    virtual Path get_module_path(const String& sub_path = "") const = 0;

    Shared<Texture> load_texture(const Name& name);
    Shared<Texture> create_texture(const Array2D<Color>& pixels, const Name& name);

    Shared<Material> load_material(const Name& name);
    Shared<Material> clone_material(const Shared<Material>& material, const Name& new_name = Name());

    Path get_resources_path() const;
    Path get_textures_path() const;
    Path get_materials_path() const;
    Path get_shaders_path() const;

private:
    void add_resource_directories();

    void register_resource_directories();
    void unregister_resource_directories();

    static void reset_global_resources_directories();

    Shared<Shader> load_shader_program(const Name& name);

    Name module_name;

    Set<String> local_directories = Set<String>();
    inline static Set<String> global_directories = Set<String>();
    inline static Set<String> global_directories_default = {
        "textures",
        "shaders",
        "materials",
        "meshes",
        "audio",
        "fonts"
    };

    Map<Name, Shared<Texture>> textures_;
    Map<Name, Shared<Material>> materials_;
    Map<Name, Shared<Shader>> shader_;
};
