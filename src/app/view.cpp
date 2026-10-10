#include "view.hpp"

#include "palette.hpp"

#include <algorithm>
#include <string>

namespace invaders::app {

using gamelib::core::Cell;
using gamelib::core::Color;
using gamelib::core::Rect;
using gamelib::core::Vec2;

namespace {

constexpr float kAlienGlow = 12.0f;
constexpr float kCannonGlow = 14.0f;
constexpr float kSaucerGlow = 16.0f;
constexpr float kBombGlow = 10.0f;
constexpr float kCanvasWidth = 1920.0f;
constexpr float kCanvasHeight = 1080.0f;

[[nodiscard]] Rect _moved(const Rect& rect, const Vec2 offset)
{
    return {rect.x + offset.x, rect.y + offset.y, rect.width, rect.height};
}

[[nodiscard]] Color _alienColor(const game::AlienType type)
{
    switch (type) {
    case game::AlienType::Squid: return palette().squid;
    case game::AlienType::Crab: return palette().crab;
    case game::AlienType::Octopus: return palette().octopus;
    }
    return palette().crab;
}

void _drawShields(gamelib::draw::DrawList& list, const ViewState& view)
{
    const Palette& colors = palette();
    for (const game::Shield& shield : view.session.shields()) {
        const float left = static_cast<float>(shield.solidCount()) / 300.0f;
        list.box(_moved(shield.bounds(), view.shake), {0.0f, 0.0f, 0.0f, 0.0f},
                 {.cornerRadius = 10.0f, .glowWidth = 22.0f, .glowColor = colors.shield.withAlpha(0.16f * std::min(left, 1.0f))});
        // One sprite per row: two bits of strength per cell.
        for (int y = 0; y < game::Shield::kHeight; ++y) {
            std::uint64_t bits = 0;
            for (int x = 0; x < game::Shield::kWidth; ++x) {
                bits |= static_cast<std::uint64_t>(shield.cells().at({x, y})) << (2 * x);
            }
            if (bits == 0) {
                continue;
            }
            const Rect first = shield.cellRect({0, y});
            list.add({.rect = _moved({first.x, first.y, first.width * game::Shield::kWidth, first.height}, view.shake),
                      .color = colors.shield,
                      .color2 = colors.shieldDim,
                      .shape = view.shapes.shield,
                      .params = {shield.cellSize(), static_cast<float>(y), 0.0f, 0.0f},
                      .bits = bits});
        }
    }
}

void _drawFormation(gamelib::draw::DrawList& list, const ViewState& view)
{
    const game::Formation& formation = view.session.formation();
    for (int row = 0; row < game::Rules::kRows; ++row) {
        const game::AlienType type = game::Formation::type(row);
        const Color color = _alienColor(type);
        for (int column = 0; column < game::Rules::kColumns; ++column) {
            if (!formation.alive({column, row})) {
                continue;
            }
            list.add({.rect = _moved(formation.rect({column, row}).expanded(kAlienGlow), view.shake),
                      .color = color,
                      .color2 = color,
                      .shape = view.shapes.alien,
                      .params = {static_cast<float>(type), static_cast<float>(formation.frame()), kAlienGlow, 0.0f}});
        }
    }
}

} // namespace

void drawCannon(gamelib::draw::DrawList& list, const Shapes& shapes, const Vec2 center, const float width,
                const float flash, const float alpha)
{
    const Palette& colors = palette();
    const float glow = kCannonGlow * width / 78.0f;
    const Vec2 size{width + 2.0f * glow, width * 44.0f / 78.0f + 2.0f * glow};
    list.add({.rect = Rect::fromCenter(center, size),
              .color = colors.cannon.withAlpha(alpha),
              .color2 = colors.cannonGlow.withAlpha(alpha),
              .shape = shapes.cannon,
              .params = {glow, flash, width, 0.0f}});
}

void drawBackground(gamelib::draw::DrawList& list, const ViewState& view)
{
    const game::Rules& rules = view.session.rules();
    list.add({.rect = {0.0f, 0.0f, kCanvasWidth, kCanvasHeight},
              .color = palette().screen,
              .color2 = palette().screenGlow,
              .shape = view.shapes.crt,
              .params = {0.0f, rules.shieldTop - 30.0f, rules.saucerY + 50.0f, 0.0f}});
}

void drawField(gamelib::draw::DrawList& list, const ViewState& view)
{
    const game::Session& session = view.session;
    const game::Rules& rules = session.rules();
    const Palette& colors = palette();

    list.box(_moved({rules.left, rules.ground, rules.right - rules.left, 4.0f}, view.shake), colors.ground,
             {.cornerRadius = 2.0f, .glowWidth = 10.0f, .glowColor = colors.ground.withAlpha(0.5f)});
    _drawShields(list, view);
    _drawFormation(list, view);

    if (const auto& saucer = session.saucer(); saucer) {
        const Vec2 at = saucer->position.at(view.alpha) + view.shake;
        list.add({.rect = Rect::fromCenter(at, {rules.saucerWidth + 2.0f * kSaucerGlow, rules.saucerHeight + 2.0f * kSaucerGlow}),
                  .color = colors.saucer,
                  .color2 = colors.saucerGlow,
                  .shape = view.shapes.saucer,
                  .params = {kSaucerGlow, 0.0f, view.time * 9.0f, 0.0f}});
    }
    for (const game::Bomb& bomb : session.bombs()) {
        const Vec2 at = bomb.position.at(view.alpha) + view.shake;
        list.add({.rect = Rect::fromCenter(at, {rules.bombWidth + 2.0f * kBombGlow, rules.bombHeight + 2.0f * kBombGlow}),
                  .color = colors.bomb,
                  .color2 = colors.bombGlow,
                  .shape = view.shapes.bomb,
                  .params = {static_cast<float>(bomb.type), at.y / 70.0f, kBombGlow, 0.0f}});
    }
    if (const auto& shot = session.shot(); shot) {
        list.box(Rect::fromCenter(shot->at(view.alpha) + view.shake, {rules.shotWidth, rules.shotHeight}), colors.shot,
                 {.cornerRadius = 3.0f, .glowWidth = 10.0f, .glowColor = colors.cannonGlow.withAlpha(0.7f)});
    }
    if (session.cannonVisible()) {
        drawCannon(list, view.shapes, Vec2{session.cannonX().at(view.alpha), rules.cannonY} + view.shake, rules.cannonWidth);
    }
}

void drawHud(gamelib::draw::DrawList& list, const ViewState& view)
{
    const game::Session& session = view.session;
    const game::Rules& rules = session.rules();
    const Palette& colors = palette();
    if (!view.hud) {
        return;
    }
    const gamelib::draw::TextStyle label{.pixelSize = 3.5f, .color = colors.textDim};
    const gamelib::draw::DigitStyle digits{.height = 46.0f,
                                           .color = colors.text,
                                           .glowColor = colors.crab.withAlpha(0.35f),
                                           .glowWidth = 10.0f};
    list.text("SCORE", rules.left, 24.0f, label);
    list.number(session.score(), rules.left, 52.0f, digits);
    list.text("HI-SCORE", 960.0f, 24.0f, {.pixelSize = 3.5f, .color = colors.textDim, .align = gamelib::draw::Align::Center});
    gamelib::draw::DigitStyle best = digits;
    best.align = gamelib::draw::Align::Center;
    best.glowColor = colors.accent.withAlpha(0.3f);
    list.number(view.best, 960.0f, 52.0f, best);
    list.text("WAVE", rules.right, 24.0f, {.pixelSize = 3.5f, .color = colors.textDim, .align = gamelib::draw::Align::Right});
    gamelib::draw::DigitStyle wave = digits;
    wave.align = gamelib::draw::Align::Right;
    list.number(session.wave(), rules.right, 52.0f, wave);

    // Lives under the ground line: the count, then the spare cannons.
    const float y = rules.ground + 44.0f;
    list.number(session.lives(), rules.left, y - 20.0f, {.height = 40.0f, .color = colors.cannon, .glowColor = colors.cannonGlow.withAlpha(0.4f), .glowWidth = 8.0f});
    for (int i = 0; i < std::min(session.lives() - 1, 6); ++i) {
        drawCannon(list, view.shapes, {rules.left + 70.0f + static_cast<float>(i) * 62.0f, y}, 46.0f);
    }
}

void drawOverlay(gamelib::draw::DrawList& list, const ViewState& view)
{
    list.add({.rect = {0.0f, 0.0f, kCanvasWidth, kCanvasHeight}, .shape = view.shapes.crt, .params = {1.0f, 0.0f, 0.0f, 0.0f}});
}

} // namespace invaders::app
