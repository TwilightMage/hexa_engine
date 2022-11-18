#pragma once

#include "ITool.h"

namespace Tools {
    class Help : public ITool {
        Name get_tool_name() const override { return Name("help"); }
        String get_tool_description() const override { return "List all available commands"; }

        void execute(const List<String>& args) override;
    };
} // namespace Tools
