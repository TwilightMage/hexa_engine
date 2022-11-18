#pragma once

#include <base_lib/Name.h>
#include <base_lib/String.h>

class ITool {
public:
    virtual Name get_tool_name() const = 0;
    virtual String get_tool_description() const = 0;

    virtual void execute(const List<String>& args) = 0;
};
