#pragma once

#include "formation.hpp"
#include "rules.hpp"
#include "shield.hpp"

#include <gamelib/core/event_queue.hpp>
#include <gamelib/core/interpolated.hpp>
#include <gamelib/core/math.hpp>
#include <gamelib/core/rng.hpp>
#include <gamelib/core/timers.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace invaders::game {

// What the player (or the demo gunner) does this tick.
struct Input {
    float move = 0.0f; // -1 left .. +1 right
    bool fire = false; // fires when no shot of the cannon is in the air
};

enum class Phase {
    Playing,
    Dying,     // the cannon is exploding; everything else holds still
    WaveClear, // the field is empty; the next wave marches in after a moment
    GameOver,
};

enum class EventKind {
    Fired,           // position: the shot's start
    ShotMissed,      // position: where it left the top of the field
    AlienKilled,     // position, alien, points
    BombDropped,     // position, bomb
    ShotHitBomb,     // position
    ShieldHit,       // position; value 1 = by a bomb, 0 = by the cannon's shot
    BombLanded,      // position: on the ground
    Step,            // value: the march note 0..3
    SaucerAppeared,  // position
    SaucerKilled,    // position, points
    SaucerLeft,
    CannonDestroyed, // position
    Respawned,
    ExtraLife,
    Invaded,         // the aliens reached the cannon's row: the game ends
    WaveCleared,
    WaveStarted,     // value: the wave number
    GameOver,
};

struct Event {
    EventKind kind;
    gamelib::core::Vec2 position{};
    int points = 0;
    int value = 0;
    AlienType alien = AlienType::Squid;
    BombType bomb = BombType::Rolling;
};

struct Bomb {
    BombType type = BombType::Rolling;
    gamelib::core::Interpolated<gamelib::core::Vec2> position; // the bomb's center
};

struct Saucer {
    gamelib::core::Interpolated<gamelib::core::Vec2> position;
    float direction = 1.0f;
};

// One game of Invaders: the formation, its bombs, four shields, the mystery
// ship and the cannon. Advances only through tick(); deterministic for a
// seed and the inputs. Presentation reads the state and takes the events.
class Session {
public:
    Session(std::uint64_t seed, float tickSeconds, Rules rules = {});

    void tick(const Input& input);

    [[nodiscard]] const Rules& rules() const { return m_rules; }
    [[nodiscard]] Phase phase() const { return m_phase; }
    [[nodiscard]] const Formation& formation() const { return m_formation; }
    [[nodiscard]] const std::array<Shield, Rules::kShields>& shields() const { return m_shields; }
    [[nodiscard]] const gamelib::core::Interpolated<float>& cannonX() const { return m_cannonX; }
    [[nodiscard]] bool cannonVisible() const { return m_phase == Phase::Playing || m_phase == Phase::WaveClear; }
    [[nodiscard]] const std::optional<gamelib::core::Interpolated<gamelib::core::Vec2>>& shot() const { return m_shot; }
    [[nodiscard]] const std::vector<Bomb>& bombs() const { return m_bombs; }
    [[nodiscard]] const std::optional<Saucer>& saucer() const { return m_saucer; }
    [[nodiscard]] int score() const { return m_score; }
    [[nodiscard]] int lives() const { return m_lives; }
    [[nodiscard]] int wave() const { return m_wave; }
    [[nodiscard]] float bombSpeed() const;
    [[nodiscard]] float tickSeconds() const { return m_dt; }
    // The interval between march steps right now, in ticks.
    [[nodiscard]] int stepTicks() const;

    [[nodiscard]] std::vector<Event> takeEvents() { return m_events.take(); }

private:
    void startWave();
    void moveCannon(const Input& input);
    void moveShot();
    void moveBombs();
    void moveSaucer();
    void march();
    void dropBombs();
    void killCannon();
    void addScore(int points);
    void finishPhase();
    [[nodiscard]] bool shotHits(gamelib::core::Vec2 at);
    [[nodiscard]] Shield* shieldAt(gamelib::core::Vec2 point);
    [[nodiscard]] gamelib::core::Rect cannonRect() const;
    [[nodiscard]] int ticks(float seconds) const { return gamelib::core::Countdown::ticksFor(seconds, m_dt); }

    Rules m_rules;
    float m_dt;
    gamelib::core::Rng m_rng;
    gamelib::core::EventQueue<Event> m_events;
    Phase m_phase = Phase::Playing;
    gamelib::core::Countdown m_phaseTimer; // Dying and WaveClear

    Formation m_formation;
    std::array<Shield, Rules::kShields> m_shields;
    gamelib::core::Interpolated<float> m_cannonX;
    std::optional<gamelib::core::Interpolated<gamelib::core::Vec2>> m_shot;
    std::vector<Bomb> m_bombs;
    std::optional<Saucer> m_saucer;

    int m_sinceStep = 0; // ticks since the last march step
    gamelib::core::Countdown m_bombTimer;
    gamelib::core::Countdown m_saucerTimer;
    int m_note = 0;      // the march's next note
    int m_nextBomb = 0;  // bomb types take turns
    int m_shotsFired = 0;

    int m_score = 0;
    int m_lives = 0;
    int m_wave = 0;
    bool m_extraLifeGiven = false;
};

} // namespace invaders::game
