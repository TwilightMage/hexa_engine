#pragma once

#include "ITool.h"

namespace Tools {
    class Comp : public ITool {
        Name get_tool_name() const override { return "comp"; }
        String get_tool_description() const override { return "convert compound-supported format to .comp format"; }

        void execute(const List<String>& args) override;
    };
} // namespace Tools