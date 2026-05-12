#pragma once

#include <memory>
#include <string>

#include "Dungeon.hpp"

namespace Parser {
struct ParseResult {
  bool ok;
  std::string bad_line;

  std::unique_ptr<Terracraft::Dungeon> dungeon;

  int M_;
};

ParseResult load_dungeon(const std::string& path);
}  // namespace Parser