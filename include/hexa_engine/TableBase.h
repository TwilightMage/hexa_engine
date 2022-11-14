#pragma once

#include "ModuleAssetID.h"

#include <base_lib/Compound.h>
#include <base_lib/Name.h>
#include <base_lib/Pointers.h>

class Game;
class Module;

class EXPORT TableBase
{
    friend Game;

public:
    virtual ~TableBase() = default;

    explicit TableBase(const String& name);

    virtual void add_record_compound(const ModuleAssetID& key, const Compound::Object& new_record, bool force_replace = false) = 0;
    virtual void add_record_compounds(const Shared<Module>& owning_module, const Compound::Object& record_compound, bool force_replace = false) = 0;

    const String& get_name() const { return name; }

private:
    virtual void post_load() {}
    virtual void init_assets() {}
    virtual void clear() {}

    String name;
};
