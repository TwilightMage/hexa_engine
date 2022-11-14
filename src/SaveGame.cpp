#include "hexa_engine/SaveGame.h"

SaveGame::SaveGame(const String& profile_name)
    : path_("savegames/" + profile_name)
{
}

const Path& SaveGame::get_path() const
{
    return path_;
}
