#include "AliceBot.hpp"

#include <algorithm>
#include <climits>
#include <queue>

namespace {

// Сколько еды нужно, чтобы вернуться отсюда в 0 по уже посещённым комнатам.
// Рёбра считаем неориентированными.
int distance_to_zero_visited(const BotView& v, int from) {
  if (from == 0) return 0;
  std::vector<std::vector<int>> adj(v.n + 1);
  for (int u = 0; u <= v.n; ++u) {
    if (v.known[u] != VISITED) continue;
    for (int x : v.neighbors[u]) {
      if (v.known[x] != VISITED) continue;
      adj[u].push_back(x);
      adj[x].push_back(u);
    }
  }
  std::vector<int> dist(v.n + 1, -1);
  std::queue<int> q;
  q.push(from);
  dist[from] = 0;
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int nb : adj[u]) {
      if (dist[nb] != -1) continue;
      dist[nb] = dist[u] + 1;
      if (nb == 0) return dist[nb];
      q.push(nb);
    }
  }
  return INT_MAX;
}

std::vector<int> shortest_path_to_zero(const BotView& v, int from) {
  std::vector<std::vector<int>> adj(v.n + 1);
  for (int u = 0; u <= v.n; ++u) {
    if (v.known[u] != VISITED) continue;
    for (int x : v.neighbors[u]) {
      if (v.known[x] != VISITED) continue;
      adj[u].push_back(x);
      adj[x].push_back(u);
    }
  }
  std::vector<int> dist(v.n + 1, -1);
  std::queue<int> q;
  dist[0] = 0;
  q.push(0);
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int nb : adj[u]) {
      if (dist[nb] != -1) continue;
      dist[nb] = dist[u] + 1;
      q.push(nb);
    }
  }
  std::vector<int> path;
  if (dist[from] == -1) return path;
  int cur = from;
  while (cur != 0) {
    int best = -1;
    for (int nb : adj[cur]) {
      if (dist[nb] != dist[cur] - 1) continue;
      if (best == -1 || nb < best) best = nb;
    }
    if (best == -1) {
      path.clear();
      return path;
    }
    path.push_back(best);
    cur = best;
  }
  return path;
}

// Самый ценный из оставшихся ресурс в комнате. Возвращает std::nullopt,
// если в комнате ничего нет.
// Берём ResourceType из BASE_VALUES, цена удваивается если это target.
struct BestResult {
  bool has;
  ResourceType type;
};

BestResult best_resource_in_room(const BotView& v, int room) {
  BestResult result{false, RES_IRON};
  long long best_val = -1;

  // Идём по ресурсам, которые есть в комнате (map итерируется в порядке
  // ключей — стабильно).
  for (const auto& [type, count] : v.res[room]) {
    if (count <= 0) continue;
    auto base_it = BASE_VALUES.find(type);
    if (base_it == BASE_VALUES.end()) continue;
    long long val = base_it->second;
    if (type == v.target) val *= 2;
    if (val > best_val) {
      best_val = val;
      result = {true, type};
    }
  }
  return result;
}

int next_explore_target(const BotView& v) {
  int cur = v.current;
  // (1) Среди смежных непосещённых — с наименьшим номером.
  int best = -1;
  for (int nb : v.neighbors[cur]) {
    if (v.known[nb] == VISITED) continue;
    if (v.known[nb] == UNKNOWN) continue;
    if (best == -1 || nb < best) best = nb;
  }
  if (best != -1) return best;

  // (2) Ищем ближайшую непосещённую через граф посещённых.
  std::vector<int> dist(v.n + 1, -1);
  std::queue<int> q;
  dist[cur] = 0;
  q.push(cur);
  int best_dist = INT_MAX, best_room = -1;
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int nb : v.neighbors[u]) {
      if (v.known[nb] == VISITED || v.known[nb] == UNKNOWN) continue;
      int d_to_nb = dist[u] + 1;
      if (d_to_nb < best_dist || (d_to_nb == best_dist && nb < best_room)) {
        best_dist = d_to_nb;
        best_room = nb;
      }
    }
    for (int nb : v.neighbors[u]) {
      if (v.known[nb] != VISITED) continue;
      if (dist[nb] != -1) continue;
      dist[nb] = dist[u] + 1;
      q.push(nb);
    }
  }
  if (best_room == -1) return -1;
  std::vector<int> parent(v.n + 1, -1);
  std::vector<int> d2(v.n + 1, -1);
  d2[cur] = 0;
  std::queue<int> q2;
  q2.push(cur);
  while (!q2.empty()) {
    int u = q2.front();
    q2.pop();
    if (u == best_room) break;
    for (int nb : v.neighbors[u]) {
      if (d2[nb] != -1) continue;
      if (nb != best_room && v.known[nb] != VISITED) continue;
      d2[nb] = d2[u] + 1;
      parent[nb] = u;
      q2.push(nb);
    }
  }
  if (d2[best_room] == -1) return -1;
  int step = best_room;
  while (parent[step] != cur) step = parent[step];
  return step;
}

}  // namespace

Action AliceBot::decide(const BotView& v) {
  ensure_sized(v.n);

  // === Фаза возврата ===
  if (returning_) {
    if (v.current == 0) return Action{ACT_STOP, 0, RES_IRON};

    int to_home = distance_to_zero_visited(v, v.current);

    if (v.food_left > to_home) {
      BestResult b = best_resource_in_room(v, v.current);
      if (b.has) return Action{ACT_COLLECT, 0, b.type};
    }

    if (return_idx_ >= (int)return_path_.size()) {
      return_path_ = shortest_path_to_zero(v, v.current);
      return_idx_ = 0;
      if (return_path_.empty()) return Action{ACT_STOP, 0, RES_IRON};
    }
    int next = return_path_[return_idx_++];
    return Action{ACT_MOVE, next, RES_IRON};
  }

  // === Фаза исследования ===
  int spent = v.food_total - v.food_left;
  bool budget_exhausted = (spent >= v.food_total / 2);

  if (!collected_here_[v.current]) {
    BestResult b = best_resource_in_room(v, v.current);
    if (b.has) {
      collected_here_[v.current] = true;
      return Action{ACT_COLLECT, 0, b.type};
    }
    collected_here_[v.current] = true;
  }

  if (budget_exhausted) {
    returning_ = true;
    return_path_ = shortest_path_to_zero(v, v.current);
    return_idx_ = 0;
    if (v.current == 0) return Action{ACT_STOP, 0, RES_IRON};
    if (return_path_.empty()) return Action{ACT_STOP, 0, RES_IRON};

    int to_home = distance_to_zero_visited(v, v.current);
    if (v.food_left > to_home) {
      BestResult b = best_resource_in_room(v, v.current);
      if (b.has) return Action{ACT_COLLECT, 0, b.type};
    }
    int next = return_path_[return_idx_++];
    return Action{ACT_MOVE, next, RES_IRON};
  }

  int next = next_explore_target(v);
  if (next == -1) {
    returning_ = true;
    return_path_ = shortest_path_to_zero(v, v.current);
    return_idx_ = 0;
    if (return_path_.empty()) return Action{ACT_STOP, 0, RES_IRON};
    int step = return_path_[return_idx_++];
    return Action{ACT_MOVE, step, RES_IRON};
  }
  return Action{ACT_MOVE, next, RES_IRON};
}