#include "view.hpp"

#include "palette.hpp"

#include <string>

namespace invaders::app {

using gamelib::core::Cell;
using gamelib::core::Color;
using gamelib::core::Rect;
using gamelib::core::Vec2;

namespace {

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

} // namespace

void drawBackground(gamelib::draw::DrawList& list, const ViewState&)
{
    list.vignette({0.0f, 0.0f, 1920.0f, 1080.0f}, palette().screenGlow, 1.0f, 0.2f);
}

void drawField(gamelib::draw::DrawList& list, const ViewState& view)
{
    const game::Session& session = view.session;
    const game::Rules& rules = session.rules();
    const Palette& colors = palette();

    list.box(_moved({rules.left, rules.ground, rules.right - rules.left, 3.0f}, view.shake), colors.ground);

    for (const game::Shield& shield : session.shields()) {
        for (int y = 0; y < game::Shield::kHeight; ++y) {
            for (int x = 0; x < game::Shield::kWidth; ++x) {
                const std::uint8_t strength = shield.cells().at({x, y});
                if (strength > 0) {
                    list.box(_moved(shield.cellRect({x, y}), view.shake),
                             strength == game::Shield::kFull ? colors.shield : colors.shieldDim);
                }
            }
        }
    }

    const game::Formation& formation = session.formation();
    for (int row = 0; row < game::Rules::kRows; ++row) {
        for (int column = 0; column < game::Rules::kColumns; ++column) {
            if (formation.alive({column, row})) {
                list.box(_moved(formation.rect({column, row}), view.shake), _alienColor(game::Formation::type(row)),
                         {.cornerRadius = 8.0f});
            }
        }
    }

    if (const auto& saucer = session.saucer(); saucer) {
        list.box(_moved(Rect::fromCenter(saucer->position.at(view.alpha), {rules.saucerWidth, rules.saucerHeight}), view.shake),
                 colors.saucer, {.cornerRadius = 18.0f});
    }
    for (const game::Bomb& bomb : session.bombs()) {
        list.box(_moved(Rect::fromCenter(bomb.position.at(view.alpha), {rules.bombWidth, rules.bombHeight}), view.shake),
                 colors.bomb);
    }
    if (const auto& shot = session.shot(); shot) {
        list.box(_moved(Rect::fromCenter(shot->at(view.alpha), {rules.shotWidth, rules.shotHeight}), view.shake), colors.shot);
    }
    if (session.cannonVisible()) {
        const Vec2 cannon{session.cannonX().at(view.alpha), rules.cannonY};
        list.box(_moved(Rect::fromCenter(cannon, {rules.cannonWidth, rules.cannonHeight}), view.shake), colors.cannon,
                 {.cornerRadius = 6.0f});
    }
}

void drawHud(gamelib::draw::DrawList& list, const ViewState& view)
{
    const game::Session& session = view.session;
    const Palette& colors = palette();
    if (!view.hud) {
        return;
    }
    list.text("SCORE " + std::to_string(session.score()), 360.0f, 40.0f, {.pixelSize = 5.0f, .color = colors.text});
    list.text("HI " + std::to_string(view.best), 960.0f, 40.0f,
              {.pixelSize = 5.0f, .color = colors.text, .align = gamelib::draw::Align::Center});
    list.text("WAVE " + std::to_string(session.wave()), 1560.0f, 40.0f,
              {.pixelSize = 5.0f, .color = colors.text, .align = gamelib::draw::Align::Right});
    list.text("LIVES " + std::to_string(session.lives()), 360.0f, 1010.0f, {.pixelSize = 4.0f, .color = colors.text});
}

} // namespace invaders::app
