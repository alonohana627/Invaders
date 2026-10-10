#pragma once

#include "session.hpp"

#include <gamelib/ai/opponent.hpp>
#include <gamelib/core/rng.hpp>

#include <cstdint>

namespace invaders::game {

// The computer gunner of the attract demo and of --demo: sidesteps bombs it
// sees coming, leads the mystery ship, and otherwise picks off the lowest
// aliens near it. It re-plans only every few ticks, aims with an error and
// sees bombs with a varying warning, so it loses after a few minutes (median
// 3 minutes over 30 seeds, measured headless) and the demo restarts. It
// avoids shooting through its own shields when a clear column is near.
class Gunner {
public:
    explicit Gunner(std::uint64_t seed);

    [[nodiscard]] Input decide(const Session& session);

private:
    [[nodiscard]] float plan(const Session& session);
    // True if a shield cell is in the way of a shot fired at x.
    [[nodiscard]] static bool shielded(const Session& session, float x);

    gamelib::core::Rng m_rng;
    gamelib::ai::ReactionTimer m_reaction{9};
    gamelib::ai::AimError m_aim;
    float m_target = 960.0f;
    float m_warning = 0.6f; // seconds of warning it needs to see a bomb, re-rolled per plan
    bool m_dodging = false;
    int m_aimKey = 0;
    bool m_hadShot = false;
    bool m_throughShield = false; // no clear column near the target: fire anyway
};

} // namespace invaders::game
