#pragma once

#include <map>
#include <vector>

#include "Resources.hpp"

namespace Bot {

enum Knowledge { NUMBERED, VISIBLE, VISITED };

struct BotView final {
  int current;
  int food_left;
  int food_total;
  ResourceType target;

  // Status of every known room
  std::map<int, Knowledge> known;

  // Known neighbors of VISIBLE and VISITED
  std::map<int, std::vector<int>> neighbors;

  // Resources of VISITED rooms
  std::map<int, std::map<ResourceType, int>> res;
};

enum ActionKind { ACT_MOVE, ACT_COLLECT, ACT_STOP };

struct Action final {
  ActionKind kind;
  int target_room;
  ResourceType resource;
};

class IBot {
 public:
  virtual ~IBot() = default;
  virtual Action decide(const BotView& v) = 0;
};
}  // namespace Bot