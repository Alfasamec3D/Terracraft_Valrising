#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "Resources.hpp"

struct Room {
  int id;
  std::vector<int> neighbors;
  // Счётчики по типу ресурса. Парсер всегда заполняет все 4 ключа.
  std::map<ResourceType, int> resources;
};

struct Dungeon {
  int n;                    // количество комнат, не считая нулевую
  std::vector<Room> rooms;  // размер n+1, индекс = номер комнаты
  int food;                 // M
  ResourceType target;      // целевой ресурс с удвоенной ценностью
};

struct ParseResult {
  bool ok;
  std::string bad_line;  // строка с ошибкой (как во входном файле)
};

// Читает подземелье из файла. При ошибке заполняет bad_line.
ParseResult load_dungeon(const std::string& path, Dungeon& out);