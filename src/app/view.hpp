#pragma once

#include "session.hpp"

#include <gamelib/core/math.hpp>
#include <gamelib/draw/draw_list.hpp>

#include <cstdint>

namespace invaders::app {

// What the view needs besides the session: interpolation and HUD values
// that live in the app.
struct ViewState {
    const game::Session& session;
    float alpha = 1.0f;        // interpolation between the last two ticks
    float time = 0.0f;         // seconds, for animation
    gamelib::core::Vec2 shake; // added to everything on the field
    std::int64_t best = 0;
    bool hud = true;           // the title's attract demo shows no score line
};

void drawBackground(gamelib::draw::DrawList& list, const ViewState& view);
// Shields, formation, saucer, bombs, the shot and the cannon.
void drawField(gamelib::draw::DrawList& list, const ViewState& view);
// Score, best, wave and lives.
void drawHud(gamelib::draw::DrawList& list, const ViewState& view);

} // namespace invaders::app
