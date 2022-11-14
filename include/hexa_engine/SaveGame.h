#pragma once

#include <base_lib/Path.h>

class String;

class EXPORT SaveGame
{
public:
    explicit SaveGame(const String& profile_name);

    virtual ~SaveGame(){};

    const Path& get_path() const;

private:
    Path path_;
};
