#include "Simulator.hpp"

#include <algorithm>
#include <map>
#include <ostream>

namespace {

// Обновляет известность для комнаты, в которую бот «зашёл».
// По правилам:
//   - сама комната становится VISITED (ресурсы открыты).
//   - каждый её сосед становится не ниже VISIBLE (видны его проходы).
//   - каждый сосед соседа — не ниже NUMBERED (известен номер).
void reveal_on_enter(BotView& v, const Dungeon& d, int room) {
  auto bump = [&](int x, Knowledge target) {
    if (v.known[x] < target) v.known[x] = target;
  };
  auto fill_neighbors = [&](int x) {
    if (v.neighbors[x].empty() && !d.rooms[x].neighbors.empty()) {
      v.neighbors[x] = d.rooms[x].neighbors;
    }
  };

  bump(room, VISITED);
  // Копируем ресурсы из подземелья — они теперь видны боту.
  v.res[room] = d.rooms[room].resources;
  fill_neighbors(room);

  for (int nb : d.rooms[room].neighbors) {
    bump(nb, VISIBLE);
    fill_neighbors(nb);
    for (int nn : d.rooms[nb].neighbors) {
      bump(nn, NUMBERED);
      // У NUMBERED видим только номер — проходы НЕ заполняем.
    }
  }
}

// Считает суммарную ценность по итогам. Целевой ресурс — удвоенная цена.
long long compute_total_value(const std::map<ResourceType, long long>& totals,
                              ResourceType target) {
  long long val = 0;
  for (const auto& [type, count] : totals) {
    auto it = BASE_VALUES.find(type);
    if (it == BASE_VALUES.end()) continue;
    long long base = it->second;
    if (type == target) base *= 2;
    val += base * count;
  }
  return val;
}

void print_result_line(std::ostream& out,
                       const std::map<ResourceType, long long>& totals,
                       long long val) {
  out << "result";
  // std::map итерируется по возрастанию ключа → iron, gold, gems, exp.
  for (const auto& [type, count] : totals) {
    (void)type;
    out << ' ' << count;
  }
  out << ' ' << val << '\n';
}

}  // namespace

long long run_simulation(const Dungeon& d, IBot& bot, std::ostream& out) {
  BotView v;
  v.n = d.n;
  v.current = 0;
  v.food_left = d.food;
  v.food_total = d.food;
  v.target = d.target;
  v.known.assign(d.n + 1, UNKNOWN);
  v.neighbors.assign(d.n + 1, {});
  v.res.assign(d.n + 1, {});  // пустые карты — комнаты ещё не VISITED

  // Флаг «было ли что собирать» — отдельно от res, для печати "_".
  // Карта на каждую комнату: ресурс → собирали ли его уже.
  std::vector<std::map<ResourceType, bool>> collected(d.n + 1);
  // Инициализируем все 4 ключа в false для каждой комнаты, чтобы
  // дальше не возиться с «есть ли ключ».
  for (auto& m : collected) {
    for (const auto& [type, _] : BASE_VALUES) m[type] = false;
  }

  // Итог. Тоже карта — но с long long, чтобы не переполнить при больших M.
  std::map<ResourceType, long long> totals;
  for (const auto& [type, _] : BASE_VALUES) totals[type] = 0;

  reveal_on_enter(v, d, 0);

  auto print_state_now = [&](int room) {
    out << "state " << room;
    // Идём по тем ресурсам, что есть в комнате (карта итерируется по
    // возрастанию ключа: iron, gold, gems, exp).
    for (const auto& [type, count] : v.res[room]) {
      out << ' ';
      if (collected[room][type])
        out << '_';
      else
        out << count;
    }
    out << '\n';
  };

  int safety_limit = 100000;
  while (safety_limit-- > 0) {
    Action a = bot.decide(v);

    if (a.kind == ACT_STOP) break;

    if (a.kind == ACT_MOVE) {
      int to = a.target_room;
      const auto& nb_cur = v.neighbors[v.current];
      bool ok_edge =
          std::find(nb_cur.begin(), nb_cur.end(), to) != nb_cur.end();
      if (!ok_edge) {
        const auto& nb_to = v.neighbors[to];
        ok_edge =
            std::find(nb_to.begin(), nb_to.end(), v.current) != nb_to.end();
      }
      if (!ok_edge) break;
      if (v.food_left <= 0 && v.current != 0) break;
      v.food_left -= 1;
      v.current = to;
      reveal_on_enter(v, d, to);
      out << "go " << to << '\n';
      if (to != 0) print_state_now(to);
      if (to == 0) {
        long long val = compute_total_value(totals, d.target);
        print_result_line(out, totals, val);
        return val;
      }
      continue;
    }

    if (a.kind == ACT_COLLECT) {
      int room = v.current;
      ResourceType k = a.resource;
      auto it = v.res[room].find(k);
      if (it == v.res[room].end() || it->second <= 0) break;
      int amount = it->second;

      // Если в этой комнате уже собирали хоть что-то — платим 1 еды.
      bool any_before = false;
      for (const auto& [type, was] : collected[room]) {
        (void)type;
        if (was) {
          any_before = true;
          break;
        }
      }
      if (any_before) {
        if (v.food_left <= 0) break;
        v.food_left -= 1;
      }
      totals[k] += amount;
      it->second = 0;
      collected[room][k] = true;

      auto name_it = RES_NAMES.find(k);
      out << "collect "
          << (name_it != RES_NAMES.end() ? name_it->second : std::string("?"))
          << '\n';
      print_state_now(room);
      continue;
    }
    break;
  }

  // Аварийная печать, если бот сам остановился где-то.
  if (v.current == 0) {
    long long val = compute_total_value(totals, d.target);
    print_result_line(out, totals, val);
    return val;
  }
  return 0;
}