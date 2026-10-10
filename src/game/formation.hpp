#pragma once

#include "rules.hpp"

#include <gamelib/core/grid.hpp>
#include <gamelib/core/math.hpp>

#include <array>
#include <cstddef>
#include <optional>

namespace invaders::game {

// The 11 x 5 block of aliens. It moves as one unit: a position (the center of
// the top-left alien) and a direction. Each step moves it sideways; after a
// step that brings an alien to the field's edge, the next step drops it one
// row and turns it around (dropping on the same step looks like a glitch;
// the pause before the drop is the arcade's lurch). The animation frame flips
// on every step, so the creatures dance faster as the march speeds up.
class Formation {
public:
    Formation() = default;
    Formation(const Rules& rules, float topRowY);

    [[nodiscard]] bool alive(gamelib::core::Cell cell) const { return m_alive[index(cell)]; }
    [[nodiscard]] int aliveCount() const { return m_aliveCount; }
    [[nodiscard]] static AlienType type(int row);
    [[nodiscard]] gamelib::core::Vec2 center(gamelib::core::Cell cell) const;
    [[nodiscard]] gamelib::core::Rect rect(gamelib::core::Cell cell) const;
    [[nodiscard]] int frame() const { return m_frame; }
    [[nodiscard]] float direction() const { return m_direction; }

    // One step of the march. Returns true when this step was a drop.
    bool step();
    void kill(gamelib::core::Cell cell);

    // The lowest living alien of a column, if any.
    [[nodiscard]] std::optional<gamelib::core::Cell> lowest(int column) const;
    // The bounding box of the living aliens (empty when none are left).
    [[nodiscard]] gamelib::core::Rect bounds() const;

private:
    [[nodiscard]] static std::size_t index(const gamelib::core::Cell cell)
    {
        return static_cast<std::size_t>(cell.y * Rules::kColumns + cell.x);
    }

    Rules m_rules; // a copy, so a Formation can be copied with its session
    gamelib::core::Vec2 m_origin;
    std::array<bool, Rules::kColumns * Rules::kRows> m_alive{};
    int m_aliveCount = 0;
    float m_direction = 1.0f; // +1 right, -1 left
    bool m_dropNext = false;
    int m_frame = 0;
};

} // namespace invaders::game
