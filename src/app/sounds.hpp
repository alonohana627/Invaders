#pragma once

#include <gamelib/audio/sound_bank.hpp>

#include <cstdint>
#include <vector>

namespace invaders::app {

// Every sound effect in Invaders, synthesized at startup with GameLib's
// chiptune synth.
enum class Sfx : std::uint8_t {
    March0,        // the four notes of the marching bass, one per step
    March1,
    March2,
    March3,
    Fire,          // the cannon's shot
    AlienDeath,
    CannonDeath,
    SaucerSiren,   // looped while the mystery ship flies
    SaucerDeath,
    ShieldHit,
    ShotHitBomb,
    BombLanded,
    ExtraLife,
    WaveCleared,
    WaveStart,
    GameOver,
    MenuMove,
    MenuSelect,
    MenuBack,
};
inline constexpr std::size_t kSfxCount = 19;

// Renders one effect to mono float samples, exactly as the game plays it.
[[nodiscard]] std::vector<float> renderSfx(Sfx sfx, std::uint32_t sampleRate);

// The effects, rendered once and registered with the player by GameLib's
// SoundBank, then queued (each plays once per frame) or looped by Sfx.
using Sounds = gamelib::audio::SoundBank<Sfx, kSfxCount>;

} // namespace invaders::app
