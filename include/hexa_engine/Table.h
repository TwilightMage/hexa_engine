#pragma once

#include "Module.h"
#include "TableBase.h"

#include <base_lib/Compound.h>
#include <base_lib/Map.h>
#include <base_lib/Pointers.h>
#include <base_lib/String.h>

class TableRecordBase;

template<Convertible<Compound::Object> T>
class Table : public TableBase
{
    friend Game;

public:
    T* get_record(const ModuleAssetID& key) const
    {
        return records_.find_or_default(key);
    }

    const Map<ModuleAssetID, T*>& records() const { return records_; }

    void add_record(const ModuleAssetID& id, const T& new_record, bool force_replace = false)
    {
        auto& data_record = records_[id];
        if (data_record)
        {
            if (force_replace)
            {
                print_warning("Table", "Record %s was replaced in table", id.to_string().c(), get_name().c());
            }
            else
            {
                print_error("Table", "Attempt to register record %s which already exists in tale", id.to_string().c(), get_name().c());
                return;
            }
        }

        data_record = new T(new_record);
    }

    void add_record_compound(const ModuleAssetID& id, const Compound::Object& new_record, bool force_replace = false) override
    {
        auto record = new_record;
        record["id"] = id.to_string();
        T object;
        convert(record, object);
        add_record(id, object, force_replace);
    }

    void add_record_compounds(const Shared<Module>& owning_module, const Compound::Object& record_compound, bool force_replace = false) override
    {
        for (const auto& record : record_compound)
        {
            if (record->value.get_type() != Compound::Type::Object)
                return;

            add_record_compound(ModuleAssetID(record->key, owning_module->get_module_name()), record->value.get_object(), force_replace);
        }
    }

private:
    explicit Table(const String& name)
        : TableBase(name)
    {
    }

    void post_load() override
    {
        if constexpr (std::derived_from<T, TableRecordBase>)
        {
            for (auto& record_entry : records_)
            {
                record_entry.value->post_load();
            }
        }
    }

    void init_assets() override
    {
        if constexpr (std::derived_from<T, TableRecordBase>)
        {
            for (auto& record_entry : records_)
            {
                record_entry.value->init_assets();
            }
        }
    }

    void clear() override
    {
        for (const auto& record : records_)
        {
            delete record.value;
        }
        records_.clear();
    }

    Map<ModuleAssetID, T*> records_;
};
