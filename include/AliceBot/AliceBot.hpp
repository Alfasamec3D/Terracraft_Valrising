#pragma once

#include <vector>

#include "IBot.hpp"
namespace Bot {
class AliceBot : public IBot {
 public:
  Action decide(const BotView& v) override;

 private:
  // Когда мы решили возвращаться — фиксируем путь, чтобы соблюсти правило
  // «на каждой развилке — наименьший номер» и идти строго им.
  std::vector<int> return_path_;
  int return_idx_ = 0;
  bool returning_ = false;

  // Помним, в каких комнатах уже делали «первый сбор» в фазе исследования.
  // Размер подземелья боту неизвестен, поэтому растим вектор по мере того,
  // как встречаем новые номера комнат.
  std::vector<bool> collected_here_;
  void ensure_sized(int room) {
    if ((int)collected_here_.size() <= room)
      collected_here_.resize(room + 1, false);
  }
};
}  // namespace Bot