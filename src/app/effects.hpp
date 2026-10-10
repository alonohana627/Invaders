#pragma once

#include <gamelib/core/color.hpp>
#include <gamelib/core/math.hpp>
#include <gamelib/draw/draw_list.hpp>
#include <gamelib/fx/juice.hpp>
#include <gamelib/fx/particles.hpp>
#include <gamelib/fx/popups.hpp>
#include <gamelib/fx/transients.hpp>

#include <optional>
#include <string>

namespace invaders::app {

// Invaders' visual feedback ("juice"), assembled from GameLib's fx parts:
// explosion flashes (fx::Transients), alien sparks and shield crumbs
// (fx::Particles), the shot's streak (fx::Trail), banners and the mystery
// ship's score where it was hit (fx::Popups), the screen shake, a red alarm
// when the cannon dies, a pulse of the aliens' glow on every march step and
// a screen flash for a new wave. Presentation only: it runs on real frame
// time and never touches the session.
class Effects {
public:
    void update(float seconds);
    void clear();

    // A flash: a bright core shrinking inside a ring that spreads out.
    void flash(gamelib::core::Vec2 at, gamelib::core::Color color, float size, float seconds);
    void sparks(gamelib::core::Vec2 at, gamelib::core::Color color, int count, float speed);
    // Bits of a shield flying off a hit; `down` for a bomb from above.
    void crumbs(gamelib::core::Vec2 at, gamelib::core::Color color, bool down);
    void scorePopup(gamelib::core::Vec2 at, int points, gamelib::core::Color color);
    void banner(std::string text, gamelib::core::Color color, float seconds);
    // Where the cannon's shot is this frame (nullopt: no shot, the streak ends).
    void recordShot(std::optional<gamelib::core::Vec2> at);

    // The shot's streak, drawn under the field's sprites.
    void drawUnder(gamelib::draw::DrawList& list, gamelib::core::Vec2 offset) const;
    // Flashes, sparks, crumbs and popups, over the field.
    void drawOver(gamelib::draw::DrawList& list, gamelib::core::Vec2 offset) const;

    gamelib::fx::Shake shake{0.6f, 22.0f};
    gamelib::fx::Fade alarm{1.3f};     // red tint after the cannon exploded
    gamelib::fx::Fade stepPulse{0.18f}; // the aliens' glow brightens on each step
    gamelib::fx::Fade screenFlash{0.5f}; // a new wave
    gamelib::fx::Fade wreck{1.0f};       // the exploding cannon flickers while this lasts

private:
    struct Flash {
        gamelib::core::Vec2 at;
        gamelib::core::Color color;
        float size = 30.0f;
    };

    gamelib::fx::Transients<Flash> m_flashes;
    gamelib::fx::Particles m_sparks{500, {0.0f, 260.0f}, 0.25f};
    gamelib::fx::Particles m_crumbs{600, {0.0f, 1100.0f}, 0.4f};
    gamelib::fx::Trail<6> m_shotTrail;
    gamelib::fx::Popups m_banner{80.0f, 0.6f};
    gamelib::fx::Popups m_scores{50.0f, 0.4f};
};

} // namespace invaders::app
