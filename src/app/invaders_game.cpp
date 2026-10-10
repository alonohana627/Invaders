#include "invaders_game.hpp"

#include "alien_frag.spv.hpp"
#include "bomb_frag.spv.hpp"
#include "cannon_frag.spv.hpp"
#include "crt_frag.spv.hpp"
#include "saucer_frag.spv.hpp"
#include "shield_frag.spv.hpp"

#include <gamelib/app/settings.hpp>
#include <gamelib/ui/menu.hpp>

#include <cmath>
#include <string>
#include <utility>

namespace invaders::app {

using gamelib::app::ArcadeAction;
using gamelib::app::ArcadeScreen;
using gamelib::platform::Key;

namespace {

// The cannon's explosion plays out before the game-over menu.
constexpr float kGameOverDelay = 1.2f;

constexpr const char* kTitleFooter = "MOVE: ARROWS / A D    FIRE: SPACE    PAUSE: ESC    FULLSCREEN: F11";

} // namespace

InvadersGame::InvadersGame(gamelib::app::App& app, gamelib::app::ArcadeProfileFile profile, const bool demo)
    : m_profile{std::move(profile)}
    , m_shapes{.crt = app.renderer().addShape(shaders::crt_frag),
               .alien = app.renderer().addShape(shaders::alien_frag),
               .cannon = app.renderer().addShape(shaders::cannon_frag),
               .saucer = app.renderer().addShape(shaders::saucer_frag),
               .bomb = app.renderer().addShape(shaders::bomb_frag),
               .shield = app.renderer().addShape(shaders::shield_frag)}
    , m_sounds{app.audio(), renderSfx}
    , m_attract{[tick = app.tickSeconds()](const std::uint64_t seed) {
                    return Demo{.session = game::Session{seed, tick}, .gunner = game::Gunner{seed * 31 + 7}};
                },
                [](Demo& d, float) { d.session.tick(d.gunner.decide(d.session)); },
                [](const Demo& d) { return d.session.phase() == game::Phase::GameOver; }, m_seeds.next64()}
    , m_demoMode{demo}
    , m_session{m_seeds.next64(), app.tickSeconds()}
    , m_gunner{m_seeds.next64()}
{
    m_profile.save(); // a missing file is written with the defaults, so it can be found and edited
    m_scores.loadBest(m_profile.value().best);
    gamelib::app::applySettings(app, m_profile.value().settings);
    if (demo) {
        startGame(app);
        m_menus.go(ArcadeScreen::Playing, m_time);
    }
}

void InvadersGame::startGame(gamelib::app::App& app)
{
    m_session = game::Session{m_seeds.next64(), app.tickSeconds()};
    m_gunner = game::Gunner{m_seeds.next64()};
    m_scores.reset();
    m_gameEnded = false;
    m_gameOverDelay.clear();
}

const game::Session& InvadersGame::shownSession() const
{
    return m_menus.is(ArcadeScreen::Title) ? m_attract.sim().session : m_session;
}

void InvadersGame::onKey(gamelib::app::App& app, const gamelib::platform::KeyEvent& event)
{
    const gamelib::app::ArcadeResult result = m_menus.onKey(app, event, m_time);
    if (result.changed) {
        m_profile.save();
    }
    switch (result.action) {
    case ArcadeAction::Start: startGame(app); break;
    case ArcadeAction::ToTitle: saveBest(); break;
    case ArcadeAction::Quit:
        saveBest();
        app.requestQuit();
        break;
    case ArcadeAction::Resume:
    case ArcadeAction::None: break;
    }
    // Feedback after acting, so a new volume is heard right away.
    switch (result.feedback) {
    case gamelib::ui::MenuFeedback::None: break;
    case gamelib::ui::MenuFeedback::Moved:
    case gamelib::ui::MenuFeedback::Changed: m_sounds.queue(app.sounds(), Sfx::MenuMove); break;
    case gamelib::ui::MenuFeedback::Selected: m_sounds.queue(app.sounds(), Sfx::MenuSelect); break;
    case gamelib::ui::MenuFeedback::Back: m_sounds.queue(app.sounds(), Sfx::MenuBack); break;
    }
}

game::Input InvadersGame::playerInput(const gamelib::platform::InputState& keys)
{
    // Holding fire shoots again as soon as the last shot is gone; a tap
    // shorter than a tick still fires (the loop keeps it until a tick saw it).
    return {.move = static_cast<float>(keys.axis({Key::Left, Key::A}, {Key::Right, Key::D})),
            .fire = keys.isAnyDown({Key::Space, Key::Up, Key::W}) || keys.wasAnyPressed({Key::Space, Key::Up, Key::W})};
}

void InvadersGame::tick(gamelib::app::App& app)
{
    switch (m_menus.current()) {
    case ArcadeScreen::Title: m_attract.tick(app.tickSeconds()); break;
    case ArcadeScreen::Playing:
        m_session.tick(m_demoMode ? m_gunner.decide(m_session) : playerInput(app.input()));
        break;
    case ArcadeScreen::GameOver: m_session.tick({}); break;
    case ArcadeScreen::Paused: break;
    }
}

void InvadersGame::frame(gamelib::app::App& app, const gamelib::app::FrameInfo& info)
{
    m_time = info.time;
    m_gameOverDelay.update(info.seconds);
    handleEvents(app);
    loopSounds(app);
    if (!m_demoMode) {
        m_scores.set(m_session.score()); // the demo never sets a best score
    }
    if (m_gameEnded && m_gameOverDelay.ready() && m_menus.is(ArcadeScreen::Playing)) {
        m_gameEnded = false;
        if (m_demoMode) {
            startGame(app); // endless
        } else {
            saveBest();
            m_menus.go(ArcadeScreen::GameOver, m_time);
            app.input().releaseAll();
        }
    }
}

void InvadersGame::handleEvents(gamelib::app::App& app)
{
    // The attract demo is silent and has no effects; its events are dropped.
    (void)m_attract.sim().session.takeEvents();
    gamelib::audio::SoundQueue& queue = app.sounds();
    for (const game::Event& event : m_session.takeEvents()) {
        switch (event.kind) {
        case game::EventKind::Fired: m_sounds.queue(queue, Sfx::Fire, 1.0f, m_soundRng.uniform(0.97f, 1.03f)); break;
        case game::EventKind::AlienKilled:
            m_sounds.queue(queue, Sfx::AlienDeath, 1.0f, m_soundRng.uniform(0.94f, 1.06f));
            break;
        case game::EventKind::ShieldHit: m_sounds.queue(queue, Sfx::ShieldHit, 1.0f, m_soundRng.uniform(0.9f, 1.1f)); break;
        case game::EventKind::ShotHitBomb: m_sounds.queue(queue, Sfx::ShotHitBomb); break;
        case game::EventKind::BombLanded: m_sounds.queue(queue, Sfx::BombLanded, 0.8f, m_soundRng.uniform(0.9f, 1.1f)); break;
        case game::EventKind::Step:
            m_sounds.queue(queue, static_cast<Sfx>(static_cast<int>(Sfx::March0) + event.value));
            break;
        case game::EventKind::SaucerKilled: m_sounds.queue(queue, Sfx::SaucerDeath); break;
        case game::EventKind::CannonDestroyed: m_sounds.queue(queue, Sfx::CannonDeath); break;
        case game::EventKind::ExtraLife: m_sounds.queue(queue, Sfx::ExtraLife); break;
        case game::EventKind::WaveCleared: m_sounds.queue(queue, Sfx::WaveCleared); break;
        case game::EventKind::WaveStarted:
            if (event.value > 1) {
                m_sounds.queue(queue, Sfx::WaveStart);
            }
            break;
        case game::EventKind::GameOver:
            m_sounds.queue(queue, Sfx::GameOver);
            m_gameEnded = true;
            m_gameOverDelay.start(kGameOverDelay);
            break;
        case game::EventKind::ShotMissed:
        case game::EventKind::BombDropped:
        case game::EventKind::SaucerAppeared:
        case game::EventKind::SaucerLeft:
        case game::EventKind::Respawned:
        case game::EventKind::Invaded: break;
        }
    }
}

void InvadersGame::loopSounds(gamelib::app::App& app)
{
    // The siren sounds for as long as the mystery ship flies on the playing
    // screen; it stops (fading) when the ship goes or the game pauses.
    const auto& saucer = m_session.saucer();
    if (saucer && m_menus.is(ArcadeScreen::Playing) && m_session.phase() == game::Phase::Playing) {
        // A little higher towards the middle of its run, like a doppler pass.
        const game::Rules& rules = m_session.rules();
        const float middle = (rules.left + rules.right) * 0.5f;
        const float near = 1.0f - std::abs(saucer->position.value().x - middle) / (middle - rules.left);
        m_sounds.loop(app.audio(), Sfx::SaucerSiren, 0.8f + 0.2f * near, 1.0f + 0.04f * near);
    } else {
        m_sounds.stopLoop(app.audio(), Sfx::SaucerSiren, 0.08f);
    }
}

void InvadersGame::saveBest()
{
    if (m_demoMode) {
        return;
    }
    m_profile.value().best = m_scores.best();
    m_profile.save();
}

void InvadersGame::draw(gamelib::app::App& app, gamelib::draw::DrawList& list, const gamelib::app::FrameInfo& info)
{
    const bool title = m_menus.is(ArcadeScreen::Title);
    const ViewState view{.session = shownSession(),
                         .shapes = m_shapes,
                         .alpha = m_menus.is(ArcadeScreen::Paused) ? 1.0f : info.alpha,
                         .time = info.time,
                         .best = m_scores.best(),
                         .hud = !title};
    drawBackground(list, view);
    drawField(list, view);
    drawHud(list, view);
    drawOverlay(list, view);

    const gamelib::ui::Menu* menu = m_menus.menu();
    if (menu == nullptr) {
        return;
    }
    gamelib::ui::MenuStyle style;
    std::string subtitle;
    std::string footer;
    if (title) {
        style.veil = style.veil.withAlpha(0.3f); // the attract demo shows through
        subtitle = m_scores.best() > 0 ? "BEST " + std::to_string(m_scores.best()) : "";
        footer = kTitleFooter;
    } else if (m_menus.is(ArcadeScreen::GameOver)) {
        subtitle = "SCORE " + std::to_string(m_session.score()) + "   WAVE " + std::to_string(m_session.wave());
        if (m_scores.isNewBest()) {
            subtitle = "NEW BEST! " + subtitle;
        }
    }
    gamelib::ui::drawMenu(list, *menu, app.canvas(), style, subtitle, footer);
}

} // namespace invaders::app
