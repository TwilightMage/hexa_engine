#pragma once

#include <base_lib/Name.h>
#include <base_lib/Path.h>

class EXPORT ModuleAssetID : public IConvertible<String>
{
public:
    ModuleAssetID();
    ModuleAssetID(const Name& module_name, const Name& asset_name);
    explicit ModuleAssetID(const String& string, const Name& default_module_name = Name());

    FAST_OPERATOR_TWO_LESS(ModuleAssetID, module_name, asset_name)
    FAST_OPERATOR_TWO_LESS_EQUAL(ModuleAssetID, module_name, asset_name)
    FAST_OPERATOR_TWO_EQUAL(ModuleAssetID, module_name, asset_name)
    FAST_OPERATOR_TWO_GREATER_EQUAL(ModuleAssetID, module_name, asset_name)
    FAST_OPERATOR_TWO_GREATER(ModuleAssetID, module_name, asset_name)
    FAST_OPERATOR_TWO_NOT_EQUAL(ModuleAssetID, module_name, asset_name)

    bool is_valid() const
    {
        return module_name.is_valid() && asset_name.is_valid();
    }

    String to_string() const;

    //Path evaluate_asset_path() const;
    //static ModuleAssetID evaluate_asset_id_from_path(const Path& path);

    Shared<Module> get_module_reference() const;

    void convert_to(String& to) const override;
    void convert_from(const String& from) override;

    Name module_name;
    Name asset_name;
};
