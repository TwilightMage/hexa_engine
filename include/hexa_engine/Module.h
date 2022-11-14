#pragma once

#include "ModuleAssetID.h"

#include <base_lib/Array2D.h>
#include <base_lib/Color.h>
#include <base_lib/Map.h>
#include <base_lib/Name.h>
#include <base_lib/Path.h>
#include <base_lib/Pointers.h>
#include <base_lib/Set.h>

class Texture;
class Material;
class Game;

class EXPORT Module
{
    friend Game;

public:
    explicit Module(const Name& module_name);

    virtual void on_add_resource_directories(Set<String>& local, Set<String>& global);

    FORCEINLINE const Name& get_module_name() const
    {
        return module_name;
    }
    FORCEINLINE const ModuleAssetID get_asset_id(const Name& asset_name) const
    {
        return ModuleAssetID(get_module_name(), asset_name);
    }

    virtual Path get_module_path(const String& sub_path = "") const = 0;

private:
    void add_resource_directories();

    void register_resource_directories();
    void unregister_resource_directories();

    static void reset_global_resources_directories();

    Name module_name;

    Set<String> local_directories = Set<String>();
    inline static Set<String> global_directories = Set<String>();
    inline static Set<String> global_directories_default = {
        "textures",
        "shaders",
        "materials",
        "meshes",
        "audio",
        "fonts"};
};
