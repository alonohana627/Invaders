#pragma once

#include <array>

namespace invaders::game {

// Every tuning number of the game, in canvas units and seconds (converted to
// ticks where a rule counts ticks). The field is a 1200 x 900 column in the
// middle of GameLib's 1920 x 1080 canvas; the view draws exactly these
// coordinates.
struct Rules {
    // --- Field ---------------------------------------------------------------
    float left = 360.0f;    // the formation, the cannon and the saucer stay between left and right
    float right = 1560.0f;
    float top = 120.0f;     // shots vanish above this line
    float ground = 985.0f;  // bombs burst on this line

    // --- Cannon --------------------------------------------------------------
    float cannonY = 940.0f;        // center
    float cannonWidth = 78.0f;
    float cannonHeight = 44.0f;
    float cannonSpeed = 480.0f;    // units per second: the field in 2.5 s
    float shotSpeed = 1500.0f;     // one shot at a time; reload is "wait for the hit or the miss"
    float shotWidth = 6.0f;
    float shotHeight = 26.0f;
    int lives = 3;
    int extraLifeAt = 1500;        // one extra cannon, as in the arcade
    float deathSeconds = 1.6f;     // the cannon's explosion; everything else holds still
    float waveClearSeconds = 2.2f; // the empty field before the next wave marches in
    float respawnCalmSeconds = 1.0f; // no new bomb this long after a respawn

    // --- Formation -----------------------------------------------------------
    static constexpr int kColumns = 11;
    static constexpr int kRows = 5;
    float columnSpacing = 88.0f;
    float rowSpacing = 68.0f;
    float alienHeight = 44.0f;
    std::array<float, 3> alienWidth{44.0f, 60.0f, 66.0f}; // squid, crab, octopus
    std::array<int, 3> alienPoints{30, 20, 10};
    float firstRowY = 250.0f;   // the top row's center in wave 1
    float waveDrop = 34.0f;     // each wave starts this much lower...
    int maxWaveDrops = 4;       // ...for at most this many waves, then the depth repeats
    float stepX = 12.0f;        // one step sideways
    float stepDown = 34.0f;     // one step down at an edge (on the step after touching it)
    float edgeMargin = 12.0f;   // the formation turns this far from the field's edge
    // Seconds between steps: base + perAlien * alive. 55 aliens: 0.63 s;
    // the last one steps every 2 ticks. The speed-up is the game.
    float stepBaseSeconds = 0.016f;
    float stepPerAlienSeconds = 0.0112f;

    // --- Bombs ---------------------------------------------------------------
    float bombSpeed = 330.0f;          // wave 1; +6% per wave, at most +50%
    float bombSpeedPerWave = 0.06f;
    float bombSpeedMaxBonus = 0.5f;
    float bombWidth = 14.0f;
    float bombHeight = 30.0f;
    float bombIntervalMin = 0.5f;      // seconds between bombs, wave 1
    float bombIntervalMax = 1.3f;
    float bombIntervalPerWave = 0.9f;  // the interval shrinks by this factor per wave...
    float bombIntervalFloor = 0.45f;   // ...down to this share of wave 1's

    // --- Shields -------------------------------------------------------------
    static constexpr int kShields = 4;
    float shieldTop = 790.0f;
    float shieldCell = 6.0f;           // a shield is 22 x 16 cells
    float shotBlast = 1.3f;            // cells cleared around a player shot's hit...
    float shotCrumble = 2.4f;          // ...and weakened around it
    float bombBlast = 1.8f;            // bombs dig deeper
    float bombCrumble = 3.0f;

    // --- Mystery ship --------------------------------------------------------
    float saucerY = 172.0f;
    float saucerWidth = 96.0f;
    float saucerHeight = 40.0f;
    float saucerSpeed = 230.0f;
    float saucerIntervalMin = 18.0f; // seconds between appearances
    float saucerIntervalMax = 28.0f;
    int saucerMinAliens = 8;         // never with fewer aliens left (as in the arcade)
    // The arcade's points, chosen by how many shots were fired: the 23rd
    // shot and every 15th after it is worth 300.
    std::array<int, 15> saucerPoints{100, 50, 50, 100, 150, 100, 100, 50, 300, 100, 100, 100, 50, 150, 100};

    // The bottom of the formation may not pass this line: the invasion.
    [[nodiscard]] float invasionLine() const { return cannonY - cannonHeight * 0.5f; }
};

// The three kinds of alien, top row first.
enum class AlienType { Squid, Crab, Octopus };

// The three kinds of bomb: rolling (aimed at the cannon's column), plunger and
// squiggly (from random columns). Each kind may be on screen once.
enum class BombType { Rolling, Plunger, Squiggly };
inline constexpr int kBombTypes = 3;

} // namespace invaders::game
