#include "hexa_engine/tools/Help.h"

#include "hexa_engine/Game.h"

void Tools::Help::execute(const List<String>& args) {
    List<String> lines;

    if (Game::get_instance()->get_tools().size() > 0) {
        lines += "Available commands:";
        for (const auto& pair : Game::get_instance()->get_tools()) {
            lines += String(pair.key.c()) + " " + pair.value->get_tool_description();
        }
    } else {
        lines += "No available commands!";
    }

    verbose("Help", String::join(lines, "\n"));
}
