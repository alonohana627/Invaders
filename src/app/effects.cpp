#include "effects.hpp"

#include <gamelib/core/easing.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <utility>

namespace invaders::app {

using gamelib::core::Color;
using gamelib::core::Vec2;

namespace {

constexpr gamelib::fx::ParticleStyle kSparkStyle{.aspect = 0.45f, .cornerRadius = 1.5f, .endScale = 0.3f};
// Crumbs are square bits of a shield cell.
constexpr gamelib::fx::ParticleStyle kCrumbStyle{.aspect = 1.0f, .cornerRadius = 0.5f, .endScale = 0.6f};
constexpr float kPi = std::numbers::pi_v<float>;

} // namespace

void Effects::update(const float seconds)
{
    shake.update(seconds);
    alarm.update(seconds);
    stepPulse.update(seconds);
    screenFlash.update(seconds);
    wreck.update(seconds);
    m_flashes.update(seconds);
    m_sparks.update(seconds);
    m_crumbs.update(seconds);
    m_banner.update(seconds);
    m_scores.update(seconds);
}

void Effects::clear()
{
    shake.clear();
    alarm.clear();
    stepPulse.clear();
    screenFlash.clear();
    wreck.clear();
    m_flashes.clear();
    m_sparks.clear();
    m_crumbs.clear();
    m_shotTrail.clear();
    m_banner.clear();
    m_scores.clear();
}

void Effects::flash(const Vec2 at, const Color color, const float size, const float seconds)
{
    m_flashes.add({.at = at, .color = color, .size = size}, seconds);
}

void Effects::sparks(const Vec2 at, const Color color, const int count, const float speed)
{
    m_sparks.burst(at, count,
                   {.speed = {0.3f * speed, speed}, .size = {4.0f, 8.0f}, .life = {0.25f, 0.6f}, .spin = {-8.0f, 8.0f}, .color = color});
}

void Effects::crumbs(const Vec2 at, const Color color, const bool down)
{
    // Bits fly away from the side the hit came from: up for the cannon's
    // shot (from below), down for a bomb.
    const float centre = down ? 0.5f * kPi : -0.5f * kPi;
    m_crumbs.burst(at, 9,
                   {.direction = {centre - 1.0f, centre + 1.0f},
                    .speed = {80.0f, 260.0f},
                    .size = {3.0f, 6.0f},
                    .life = {0.35f, 0.7f},
                    .spin = {-6.0f, 6.0f},
                    .color = color});
}

void Effects::scorePopup(const Vec2 at, const int points, const Color color)
{
    m_scores.add(std::to_string(points), color, 1.1f, at);
}

void Effects::banner(std::string text, const Color color, const float seconds)
{
    m_banner.add(std::move(text), color, seconds);
}

void Effects::recordShot(const std::optional<Vec2> at)
{
    if (!at) {
        m_shotTrail.clear();
        return;
    }
    m_shotTrail.record(*at);
}

void Effects::drawUnder(gamelib::draw::DrawList& list, const Vec2 offset) const
{
    // The streak: thin boxes from point to point, fainter towards the tail.
    for (std::size_t i = 1; i < m_shotTrail.size(); ++i) {
        const Vec2 a = m_shotTrail[i - 1];
        const Vec2 b = m_shotTrail[i];
        const float length = gamelib::core::length(a - b);
        if (length < 0.5f) {
            continue;
        }
        const float fade = 1.0f - gamelib::fx::Trail<6>::age(i);
        list.box(gamelib::core::Rect::fromCenter((a + b) * 0.5f + offset, {2.0f + 3.0f * fade, length + 4.0f}),
                 Color{0.6f, 1.0f, 0.75f, 0.35f * fade}, {.cornerRadius = 2.0f});
    }
}

void Effects::drawOver(gamelib::draw::DrawList& list, const Vec2 offset) const
{
    for (const auto& item : m_flashes) {
        const Flash& f = item.value;
        const float t = item.progress();
        const float fade = 1.0f - t;
        // A white-hot core that shrinks, in a ring of the object's color that spreads.
        const float core = f.size * 0.55f * (1.0f - gamelib::core::easeOutCubic(t));
        if (core > 0.5f) {
            list.circle(f.at + offset, core, gamelib::core::mix(f.color, Color{1.0f, 1.0f, 1.0f, 1.0f}, 0.7f).withAlpha(fade),
                        {.glowWidth = f.size * 0.5f, .glowColor = f.color.withAlpha(0.6f * fade)});
        }
        const float radius = f.size * (0.3f + 0.9f * gamelib::core::easeOutCubic(t));
        list.circle(f.at + offset, radius, f.color.withAlpha(fade),
                    {.ringWidth = std::max(1.0f, f.size * 0.14f * fade), .glowWidth = 10.0f, .glowColor = f.color.withAlpha(0.4f * fade)});
    }
    m_crumbs.draw(list, offset, kCrumbStyle);
    m_sparks.draw(list, offset, kSparkStyle);
    m_scores.draw(list, offset, {.pixelSize = 5.0f, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .shadow = true}, 50.0f);
    m_banner.draw(list, Vec2{960.0f, 600.0f} + offset,
                  {.pixelSize = 9.0f, .color = {1.0f, 1.0f, 1.0f, 1.0f}, .shadow = true}, 30.0f);
}

} // namespace invaders::app
