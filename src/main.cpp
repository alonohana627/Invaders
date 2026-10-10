// Invaders - a Space Invaders-style shooter on GameLib.
//
// Command line: --demo (the computer plays, endlessly) plus GameLib's
// self-test switches (--frames N --hidden --require-validation --no-vsync
// --allow-software --resize-at-frame N --minimize-at-frame N).
#include "invaders_game.hpp"

#include <gamelib/app/app.hpp>
#include <gamelib/app/arcade_profile.hpp>

#include <memory>
#include <utility>

int main(int argc, char** argv)
{
    // Loaded before the window exists, so a saved fullscreen setting is
    // applied while the window is still hidden. INVADERS_DATA_DIR moves the
    // file (the smoke runs keep theirs in the build folder).
    gamelib::app::ArcadeProfileFile profile = gamelib::app::openArcadeProfile("Invaders", "INVADERS_DATA_DIR");
    const gamelib::app::AppConfig config{
        .title = "Invaders",
        .windowSize = {1280, 720},
        .canvas = {1920, 1080},
        .tick = std::chrono::nanoseconds{1'000'000'000 / 120},
        .fullscreen = profile.value().settings.fullscreen,
        .f11TogglesFullscreen = false, // F11 goes through ArcadeMenus, so the setting is saved
        .gameFlags = {"--demo"},
    };
    return gamelib::app::runMain(argc, argv, config, [&](gamelib::app::App& app) {
        return std::make_unique<invaders::app::InvadersGame>(app, std::move(profile), app.hasFlag("--demo"));
    });
}
