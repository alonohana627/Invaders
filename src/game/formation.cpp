#include "formation.hpp"

#include <algorithm>
#include <limits>

namespace invaders::game {

using gamelib::core::Cell;
using gamelib::core::Rect;
using gamelib::core::Vec2;

Formation::Formation(const Rules& rules, const float topRowY)
    : m_rules{rules}
    , m_aliveCount{Rules::kColumns * Rules::kRows}
{
    m_alive.fill(true);
    // Centered on the field.
    const float width = static_cast<float>(Rules::kColumns - 1) * rules.columnSpacing;
    m_origin = {(rules.left + rules.right - width) * 0.5f, topRowY};
}

AlienType Formation::type(const int row)
{
    if (row == 0) {
        return AlienType::Squid;
    }
    return row <= 2 ? AlienType::Crab : AlienType::Octopus;
}

Vec2 Formation::center(const Cell cell) const
{
    return m_origin
           + Vec2{static_cast<float>(cell.x) * m_rules.columnSpacing, static_cast<float>(cell.y) * m_rules.rowSpacing};
}

Rect Formation::rect(const Cell cell) const
{
    const float width = m_rules.alienWidth[static_cast<std::size_t>(type(cell.y))];
    return Rect::fromCenter(center(cell), {width, m_rules.alienHeight});
}

bool Formation::step()
{
    m_frame ^= 1;
    if (m_dropNext) {
        m_dropNext = false;
        m_origin.y += m_rules.stepDown;
        m_direction = -m_direction;
        return true;
    }
    m_origin.x += m_direction * m_rules.stepX;
    const Rect box = bounds();
    if ((m_direction > 0.0f && box.right() >= m_rules.right - m_rules.edgeMargin)
        || (m_direction < 0.0f && box.left() <= m_rules.left + m_rules.edgeMargin)) {
        m_dropNext = true;
    }
    return false;
}

void Formation::kill(const Cell cell)
{
    if (m_alive[index(cell)]) {
        m_alive[index(cell)] = false;
        --m_aliveCount;
    }
}

std::optional<Cell> Formation::lowest(const int column) const
{
    for (int row = Rules::kRows - 1; row >= 0; --row) {
        if (alive({column, row})) {
            return Cell{column, row};
        }
    }
    return std::nullopt;
}

Rect Formation::bounds() const
{
    float left = std::numeric_limits<float>::max();
    float top = left;
    float right = std::numeric_limits<float>::lowest();
    float bottom = right;
    for (int row = 0; row < Rules::kRows; ++row) {
        for (int column = 0; column < Rules::kColumns; ++column) {
            if (!alive({column, row})) {
                continue;
            }
            const Rect r = rect({column, row});
            left = std::min(left, r.left());
            top = std::min(top, r.top());
            right = std::max(right, r.right());
            bottom = std::max(bottom, r.bottom());
        }
    }
    return m_aliveCount > 0 ? Rect{left, top, right - left, bottom - top} : Rect{};
}

} // namespace invaders::game
