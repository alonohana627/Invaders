#pragma once

#include "session.hpp"

#include <gamelib/core/math.hpp>
#include <gamelib/draw/draw_list.hpp>
#include <gamelib/draw/sprite.hpp>

#include <cstdint>

namespace invaders::app {

// Invaders' own procedural shapes (registered once with the renderer).
struct Shapes {
    gamelib::draw::ShapeId crt;    // shaders/crt.frag: background and scanline overlay
    gamelib::draw::ShapeId alien;  // shaders/alien.frag
    gamelib::draw::ShapeId cannon; // shaders/cannon.frag
    gamelib::draw::ShapeId saucer; // shaders/saucer.frag
    gamelib::draw::ShapeId bomb;   // shaders/bomb.frag
    gamelib::draw::ShapeId shield; // shaders/shield.frag
};

// What the view needs besides the session: interpolation and HUD values
// that live in the app.
struct ViewState {
    const game::Session& session;
    const Shapes& shapes;
    float alpha = 1.0f;        // interpolation between the last two ticks
    float time = 0.0f;         // seconds, for animation
    gamelib::core::Vec2 shake; // added to everything on the field
    std::int64_t best = 0;
    float pulse = 0.0f;        // 0..1 the aliens brighten on a march step
    float flash = 0.0f;        // 0..1 the screen brightens (a new wave)
    float alarm = 0.0f;        // 0..1 red tint after the cannon exploded
    float wreck = 0.0f;        // 0..1 the exploding cannon flickers while this lasts
    bool hud = true;           // the title's attract demo shows no score line
};

void drawBackground(gamelib::draw::DrawList& list, const ViewState& view);
// Ground, shields, formation, saucer, bombs, the shot and the cannon.
void drawField(gamelib::draw::DrawList& list, const ViewState& view);
// Score, best, wave and the spare cannons.
void drawHud(gamelib::draw::DrawList& list, const ViewState& view);
// Scanlines and the tube's dark corners, over everything but the menus.
void drawOverlay(gamelib::draw::DrawList& list, const ViewState& view);

// The points of each alien and the mystery ship, beside the title menu.
void drawScoreTable(gamelib::draw::DrawList& list, const ViewState& view);

// The cannon at `center` (also the spare cannons in the HUD).
void drawCannon(gamelib::draw::DrawList& list, const Shapes& shapes, gamelib::core::Vec2 center, float width,
                float flash = 0.0f, float alpha = 1.0f);

} // namespace invaders::app
