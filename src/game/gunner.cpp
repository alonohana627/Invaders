#include "gunner.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace invaders::game {

using gamelib::core::Cell;
using gamelib::core::Vec2;

Gunner::Gunner(const std::uint64_t seed)
    : m_rng{seed}
{
}

Input Gunner::decide(const Session& session)
{
    if (session.phase() != Phase::Playing && session.phase() != Phase::WaveClear) {
        m_reaction.restart();
        return {};
    }
    // A new aim error for every shot fired.
    if (session.shot().has_value() && !m_hadShot) {
        ++m_aimKey;
    }
    m_hadShot = session.shot().has_value();
    if (m_reaction.tick()) {
        m_target = plan(session);
    }
    const float x = session.cannonX().value();
    const bool aligned = std::abs(x - m_target) < 8.0f;
    return {.move = gamelib::ai::steer(x, m_target, 3.0f, 40.0f, 1.0f),
            .fire = aligned && !m_dodging && !session.shot().has_value() && (m_throughShield || !shielded(session, x))};
}

float Gunner::plan(const Session& session)
{
    const Rules& rules = session.rules();
    const float x = session.cannonX().value();
    const float half = rules.cannonWidth * 0.5f;
    const float cannonTop = rules.cannonY - rules.cannonHeight * 0.5f;
    const float bombSpeed = session.bombSpeed();
    // It sees a bomb 0.4-0.85 s before it lands (a new reaction each plan).
    m_warning = m_rng.uniform(0.4f, 0.85f);

    // Until its top has passed the cannon's bottom, a bomb can still hit.
    const float passed = -(rules.cannonHeight + rules.bombHeight) / bombSpeed;
    const float reach = half + rules.bombWidth * 0.5f + 14.0f;
    // A bomb is a threat when it will reach the cannon's row soon and is
    // over (or near) the stretch of ground the cannon covers at `at`.
    const auto threatened = [&](const float at, const float warning) {
        return std::ranges::any_of(session.bombs(), [&](const Bomb& bomb) {
            const Vec2 p = bomb.position.value();
            const float seconds = (cannonTop - (p.y + rules.bombHeight * 0.5f)) / bombSpeed;
            return seconds > passed && seconds < warning && std::abs(p.x - at) < reach;
        });
    };
    // Whether the cannon can walk from x to `to` without running into a
    // bomb that lands on the way while it passes.
    const auto clearPath = [&](const float to) {
        for (float at = x; std::abs(at - x) <= std::abs(to - x); at += (to > x ? 10.0f : -10.0f)) {
            const float when = std::abs(at - x) / rules.cannonSpeed;
            for (const Bomb& bomb : session.bombs()) {
                const Vec2 p = bomb.position.value();
                const float seconds = (cannonTop - (p.y + rules.bombHeight * 0.5f)) / bombSpeed;
                if (std::abs(p.x - at) < reach && when > seconds + passed - 0.1f && when < seconds + 0.1f) {
                    return false;
                }
            }
        }
        return true;
    };

    const float low = rules.left + half;
    const float high = rules.right - half;
    if (threatened(x, m_warning)) {
        // Step to the nearest safe spot, trying both sides.
        m_dodging = true;
        for (float offset = 40.0f; offset < 600.0f; offset += 20.0f) {
            for (const float side : {-1.0f, 1.0f}) {
                const float candidate = x + side * offset;
                if (candidate >= low && candidate <= high && !threatened(candidate, m_warning + 0.3f)
                    && clearPath(candidate)) {
                    return candidate;
                }
            }
        }
        return x;
    }
    m_dodging = false;

    const float shotSeconds = (rules.cannonY - rules.saucerY) / rules.shotSpeed;
    if (const auto& saucer = session.saucer(); saucer) {
        // Lead the mystery ship: be where it will be when the shot arrives.
        const float lead = saucer->position.value().x + saucer->direction * rules.saucerSpeed * shotSeconds;
        if (lead > low && lead < high && std::abs(lead - x) < 420.0f) {
            const float aim = lead + m_aim.update(m_aimKey, 10.0f, m_rng);
            if (!threatened(aim, m_warning + 0.2f) && clearPath(aim)) {
                return aim;
            }
        }
    }

    // The lowest aliens are the danger; among them prefer the near ones.
    const Formation& formation = session.formation();
    float best = std::numeric_limits<float>::max();
    float target = x;
    for (int column = 0; column < Rules::kColumns; ++column) {
        const std::optional<Cell> alien = formation.lowest(column);
        if (!alien) {
            continue;
        }
        const Vec2 center = formation.center(*alien);
        const float cost = std::abs(center.x - x) - 1.5f * center.y;
        if (cost < best) {
            best = cost;
            target = center.x;
        }
    }
    // The formation keeps stepping while the shot climbs: lead by the steps
    // it will take on the way.
    const float climb = (rules.cannonY - rules.firstRowY) / rules.shotSpeed;
    const float steps = climb / (static_cast<float>(session.stepTicks()) * session.tickSeconds());
    target += formation.direction() * rules.stepX * std::floor(steps);
    target += m_aim.update(m_aimKey, 14.0f, m_rng);
    target = std::clamp(target, low, high);
    // Shooting through its own shields wastes shots and the cover: step to
    // the nearest clear column within reach (the formation moves on, so a
    // target behind a shield comes into the open later).
    m_throughShield = false;
    if (shielded(session, target)) {
        float clear = target;
        for (float offset = 8.0f; offset <= 64.0f && clear == target; offset += 8.0f) {
            for (const float side : {-1.0f, 1.0f}) {
                const float candidate = std::clamp(target + side * offset, low, high);
                if (!shielded(session, candidate)) {
                    clear = candidate;
                    break;
                }
            }
        }
        m_throughShield = clear == target;
        target = clear;
    }
    // Never walk under a bomb to take a shot.
    return threatened(target, m_warning + 0.2f) || !clearPath(target) ? x : target;
}


bool Gunner::shielded(const Session& session, const float x)
{
    for (const Shield& shield : session.shields()) {
        const gamelib::core::Rect box = shield.bounds();
        if (x < box.left() || x > box.right()) {
            continue;
        }
        for (float y = box.top() + shield.cellSize() * 0.5f; y < box.bottom(); y += shield.cellSize()) {
            if (shield.solidAt({x, y})) {
                return true;
            }
        }
    }
    return false;
}
} // namespace invaders::game
