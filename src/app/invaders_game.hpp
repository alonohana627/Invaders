#pragma once

#include "effects.hpp"
#include "gunner.hpp"
#include "session.hpp"
#include "sounds.hpp"
#include "view.hpp"

#include <gamelib/ai/attract.hpp>
#include <gamelib/app/app.hpp>
#include <gamelib/app/arcade_menus.hpp>
#include <gamelib/app/arcade_profile.hpp>
#include <gamelib/core/rng.hpp>
#include <gamelib/core/score.hpp>
#include <gamelib/core/timers.hpp>

namespace invaders::app {

// The attract demo behind the title menu: a session and its computer gunner.
struct Demo {
    game::Session session;
    game::Gunner gunner;
};

// Invaders on GameLib's main loop: maps the keyboard (or the demo gunner) to
// the session's input, runs the arcade menus (title, pause, game over),
// keeps the best score and settings, and draws everything.
class InvadersGame final : public gamelib::app::Game {
public:
    // demo: the computer plays on the playing screen, endlessly (--demo).
    InvadersGame(gamelib::app::App& app, gamelib::app::ArcadeProfileFile profile, bool demo);

    void onKey(gamelib::app::App& app, const gamelib::platform::KeyEvent& event) override;
    void tick(gamelib::app::App& app) override;
    void frame(gamelib::app::App& app, const gamelib::app::FrameInfo& info) override;
    void draw(gamelib::app::App& app, gamelib::draw::DrawList& list, const gamelib::app::FrameInfo& info) override;

private:
    void startGame(gamelib::app::App& app);
    void handleEvents(gamelib::app::App& app);
    void loopSounds(gamelib::app::App& app);
    void saveBest();
    [[nodiscard]] static game::Input playerInput(const gamelib::platform::InputState& keys);
    [[nodiscard]] const game::Session& shownSession() const;

    gamelib::app::ArcadeProfileFile m_profile; // the menus edit its settings in place
    gamelib::app::ArcadeMenus m_menus{"INVADERS", m_profile.value().settings};
    Shapes m_shapes;
    Sounds m_sounds;
    Effects m_effects;
    gamelib::core::Rng m_seeds{gamelib::core::randomSeed()};
    gamelib::ai::AttractMode<Demo> m_attract;
    gamelib::core::ScoreKeeper m_scores;
    gamelib::core::Cooldown m_gameOverDelay; // the last explosion plays out before the game-over menu
    gamelib::core::Cooldown m_hitStop;       // a brief freeze when the cannon is hit
    bool m_gameEnded = false;

    bool m_demoMode;
    game::Session m_session; // the player's game (or the --demo game)
    game::Gunner m_gunner;
    float m_time = 0.0f;
    gamelib::core::Rng m_soundRng{gamelib::core::randomSeed()}; // small pitch variations
};

} // namespace invaders::app
