#pragma once

#include <iosfwd>

#include "Dungeon.hpp"
#include "IBot.hpp"
namespace Simulator {

void run_simulation(Terracraft::Dungeon& d, int food, Bot::IBot& bot,
                    std::ostream& out);
}