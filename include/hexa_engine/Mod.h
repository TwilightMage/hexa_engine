#pragma once

#include "Module.h"

#include <base_lib/Path.h>
#include <base_lib/Pointers.h>
#include <base_lib/Version.h>

class Game;
class EventBus;

class EXPORT Mod : public Module
{
    friend Game;

public:
    Mod(const Name& module_name);

    struct Info
    {
        void ReadFrom(const String& path);

        String get_full_display_name() const;

        String name;
        String display_name;
        Version mod_version;
        Version target_game_version;
    };

    Path get_module_path(const String& sub_path) const override;

    const Info& get_mod_info() const;

    static bool verify_signature(const Path& path);

    Path mod_path(const String& sub_path) const;

protected:
    virtual void on_loading_stage();
    virtual void on_start(const Shared<EventBus>& event_bus);

private:
    static Shared<Mod> load(const Path& path);
    static Info load_mod_info(const Path& path);

    Info info_;
    HINSTANCE dll_;
};

#define IMPLEMENT_MOD_ENTRY(ModTypeName) \
    extern "C" Mod EXPORT* get_mod()     \
    {                                    \
        return new ModTypeName();        \
    }