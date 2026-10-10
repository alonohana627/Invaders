#include "shield.hpp"

#include <gamelib/core/collision.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

namespace invaders::game {

using gamelib::core::Cell;
using gamelib::core::Rect;
using gamelib::core::Vec2;

namespace {

// The arcade's bunker: a flat-topped arch with a notch for the cannon.
constexpr std::array<std::string_view, Shield::kHeight> kShape{
    "    ##############    ", //
    "   ################   ", //
    "  ##################  ", //
    " #################### ", //
    "######################", //
    "######################", //
    "######################", //
    "######################", //
    "######################", //
    "######################", //
    "######################", //
    "######################", //
    "#######        #######", //
    "######          ######", //
    "#####            #####", //
    "#####            #####", //
};

} // namespace

Shield::Shield(const Vec2 topLeft, const float cellSize)
    : m_topLeft{topLeft}
    , m_cellSize{cellSize}
    , m_cells{gamelib::core::gridFromText(kShape, [](const char c) -> std::uint8_t { return c == '#' ? kFull : 0; })}
{
}

std::optional<Cell> Shield::cellAt(const Vec2 point) const
{
    const Vec2 local = (point - m_topLeft) / m_cellSize;
    const Cell cell{static_cast<int>(std::floor(local.x)), static_cast<int>(std::floor(local.y))};
    return m_cells.inside(cell) ? std::optional{cell} : std::nullopt;
}

bool Shield::solidAt(const Vec2 point) const
{
    const std::optional<Cell> cell = cellAt(point);
    return cell && m_cells.at(*cell) > 0;
}

int Shield::blast(const Vec2 point, const float blast, const float crumble, gamelib::core::Rng& rng)
{
    const Vec2 local = (point - m_topLeft) / m_cellSize;
    const int reach = static_cast<int>(std::ceil(crumble));
    const Cell center{static_cast<int>(std::floor(local.x)), static_cast<int>(std::floor(local.y))};
    int cleared = 0;
    for (int y = center.y - reach; y <= center.y + reach; ++y) {
        for (int x = center.x - reach; x <= center.x + reach; ++x) {
            const Cell cell{x, y};
            if (!m_cells.inside(cell) || m_cells.at(cell) == 0) {
                continue;
            }
            const Vec2 middle{static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};
            const float distance = gamelib::core::length(middle - local);
            std::uint8_t& strength = m_cells.at(cell);
            if (distance <= blast) {
                strength = 0;
            } else if (distance <= crumble) {
                // The closer to the blast, the likelier a cell goes or weakens.
                const float closeness = 1.0f - (distance - blast) / (crumble - blast);
                if (rng.chance(0.65f * closeness)) {
                    strength = 0;
                } else if (rng.chance(0.6f)) {
                    --strength;
                }
            } else {
                continue;
            }
            if (strength == 0) {
                ++cleared;
            }
        }
    }
    return cleared;
}

int Shield::erase(const Rect& rect)
{
    int cleared = 0;
    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            std::uint8_t& strength = m_cells.at({x, y});
            if (strength > 0 && gamelib::core::overlaps(rect, cellRect({x, y}))) {
                strength = 0;
                ++cleared;
            }
        }
    }
    return cleared;
}

Rect Shield::bounds() const
{
    return {m_topLeft.x, m_topLeft.y, m_cellSize * kWidth, m_cellSize * kHeight};
}

Rect Shield::cellRect(const Cell cell) const
{
    return {m_topLeft.x + static_cast<float>(cell.x) * m_cellSize, m_topLeft.y + static_cast<float>(cell.y) * m_cellSize,
            m_cellSize, m_cellSize};
}

int Shield::solidCount() const
{
    return static_cast<int>(std::ranges::count_if(m_cells.cells(), [](const std::uint8_t s) { return s > 0; }));
}

} // namespace invaders::game
