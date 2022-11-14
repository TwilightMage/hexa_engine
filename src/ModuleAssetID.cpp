#include "hexa_engine/ModuleAssetID.h"

#include "hexa_engine/Game.h"

ModuleAssetID::ModuleAssetID()
    : module_name()
    , asset_name()
{}

ModuleAssetID::ModuleAssetID(const Name& module_name, const Name& asset_name)
    : module_name(module_name)
    , asset_name(asset_name)
{}

ModuleAssetID::ModuleAssetID(const String& string, const Name& default_module_name)
{
    const String& src = Path::sanitize(string);

    const auto colon_pos = src.index_of(':');
    const auto slash_pos = src.index_of("/");
    if (colon_pos > -1 && (slash_pos > -1 && colon_pos < slash_pos || slash_pos == -1))
    {
        module_name = Name(src.substring(0, colon_pos));
        asset_name = Name(src.substring(colon_pos + 1));
    }
    else
    {
        module_name = Name(default_module_name);
        asset_name = Name(src);
    }
}

String ModuleAssetID::to_string() const
{
    return module_name.to_string() + ":" + asset_name.to_string();
}

Path ModuleAssetID::evaluate_asset_path() const
{
    if (auto module = Game::get_module_by_name(module_name))
    {
        return module->get_module_path("resources/" + asset_name.to_string());
    }

    return Path();
}

ModuleAssetID ModuleAssetID::evaluate_asset_id_from_path(const Path& path)
{
    const String game_path = Game::get_instance()->get_module_path().get_absolute_string();
    const String asset_path = path.get_absolute_string();

    if (asset_path.starts_with(game_path))
    {
        auto relative_path = asset_path.substring(game_path.length() + 1);

        if (relative_path.starts_with("mods/"))
        {
            relative_path = relative_path.substring(relative_path.index_of("/") + 1);

            const String module_name = relative_path.substring(relative_path.index_of("/"));
            relative_path = relative_path.substring(relative_path.index_of("/") + 1);

            if (relative_path.starts_with("resources/"))
            {
                relative_path = relative_path.substring(relative_path.index_of("/") + 1);

                return ModuleAssetID(Name(module_name), Name(relative_path));
            }
        }
        else
        {
            return ModuleAssetID(relative_path);
        }
    }

    return ModuleAssetID();
}

const Shared<Module>& ModuleAssetID::get_module_reference() const
{
    return Game::get_instance()->get_module_by_name(module_name);
}

void ModuleAssetID::convert_to(String& to) const
{
    to = to_string();
}

void ModuleAssetID::convert_from(const String& from)
{
    const String& src = Path::sanitize(from);

    const auto colon_pos = src.index_of(':');
    if (colon_pos >= 0 && src.index_of("/") > colon_pos)
    {
        module_name = Name(src.substring(0, colon_pos));
        asset_name = Name(src.substring(colon_pos + 1));
    }
    else
    {
        module_name = Name();
        asset_name = Name(src);
    }
}
