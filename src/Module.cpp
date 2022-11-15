#include "hexa_engine/Module.h"

#include "hexa_engine/Material.h"
#include "hexa_engine/Texture.h"

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreTextureManager.h>
#include <base_lib/Logger.h>

Module::Module(const Name& module_name)
    : module_name(module_name)
{
}

void Module::on_add_resource_directories(Set<String>& local, Set<String>& global)
{
}

void Module::add_resource_directories()
{
    Set<String> local;
    Set<String> global;

    on_add_resource_directories(local, global);

    local_directories += local;
    global_directories += global;
}

void Module::register_resource_directories()
{
    const Set<String> directories = local_directories + global_directories;
    for (const auto& directory : directories)
    {
        const auto path = get_module_path("resources/" + directory);
        if (path.exists())
        {
            Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(path.get_absolute_string().c(), "FileSystem", module_name.c());
        }
    }
}

void Module::unregister_resource_directories()
{
    const auto locations = Ogre::ResourceGroupManager::getSingletonPtr()->listResourceLocations(module_name.c());
    for (uint i = 0; i < locations->size(); i++)
    {
        Ogre::ResourceGroupManager::getSingletonPtr()->removeResourceLocation(locations->at(i), module_name.c());
    }
}

void Module::reset_global_resources_directories()
{
    global_directories = global_directories_default;
}
