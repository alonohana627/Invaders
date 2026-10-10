#pragma once

#include <gamelib/core/grid.hpp>
#include <gamelib/core/math.hpp>
#include <gamelib/core/rng.hpp>

#include <cstdint>
#include <optional>

namespace invaders::game {

// One destructible bunker: a 22 x 16 grid of cells, each with a strength of
// 0 (gone) to 3 (intact). Any cell above 0 stops a shot or a bomb. A hit
// clears the cells near the impact and weakens (or, by chance, clears) a
// ragged ring around them, so holes look bitten rather than stamped.
class Shield {
public:
    static constexpr int kWidth = 22;
    static constexpr int kHeight = 16;
    static constexpr std::uint8_t kFull = 3;

    Shield() = default;
    Shield(gamelib::core::Vec2 topLeft, float cellSize);

    // The cell under a canvas point, if the point is on the shield.
    [[nodiscard]] std::optional<gamelib::core::Cell> cellAt(gamelib::core::Vec2 point) const;
    [[nodiscard]] bool solidAt(gamelib::core::Vec2 point) const;
    // Clears the cells within `blast` cells of `point` and crumbles those
    // within `crumble` cells. Returns how many cells were cleared.
    int blast(gamelib::core::Vec2 point, float blast, float crumble, gamelib::core::Rng& rng);
    // Clears every cell overlapping `rect` (an alien marching through).
    int erase(const gamelib::core::Rect& rect);

    [[nodiscard]] gamelib::core::Rect bounds() const;
    [[nodiscard]] gamelib::core::Rect cellRect(gamelib::core::Cell cell) const;
    [[nodiscard]] const gamelib::core::Grid<std::uint8_t>& cells() const { return m_cells; }
    [[nodiscard]] float cellSize() const { return m_cellSize; }
    [[nodiscard]] int solidCount() const;

private:
    gamelib::core::Vec2 m_topLeft;
    float m_cellSize = 6.0f;
    gamelib::core::Grid<std::uint8_t> m_cells{0, 0};
};

} // namespace invaders::game
