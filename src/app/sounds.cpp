#include "sounds.hpp"

#include <gamelib/audio/synth.hpp>

namespace invaders::app {

namespace {

using gamelib::audio::Note;
using gamelib::audio::Wave;
using gamelib::audio::noise;
using gamelib::audio::tone;

// MIDI note numbers. The march walks down A G F E (Phrygian, menacing);
// everything else sits on E minor so the sounds belong together.
constexpr float kE2 = 40, kF2 = 41, kG2 = 43, kA2 = 45, kE3 = 52, kB3 = 59;
constexpr float kE4 = 64, kG4 = 67, kB4 = 71, kD5 = 74, kE5 = 76, kG5 = 79, kB5 = 83, kE6 = 88;
constexpr float kB6 = 95, kE7 = 100;

struct Recipe {
    std::vector<std::vector<Note>> tracks;
    float peakDb = -14.0f; // loudest sample, dBFS
};

// One march note: a low triangle thump that sags a little, with a short
// square an octave up so laptop speakers hear it too.
Recipe _march(const float note)
{
    return {{{tone(Wave::Triangle, note, 0.13f, note - 1.5f)}, {tone(Wave::Pulse50, note + 12.0f, 0.06f, note + 10.0f, 0.35f)}},
            -11.0f};
}

Recipe _recipe(const Sfx sfx)
{
    switch (sfx) {
    case Sfx::March0: return _march(kA2);
    case Sfx::March1: return _march(kG2);
    case Sfx::March2: return _march(kF2);
    case Sfx::March3: return _march(kE2);
    // The shot: a thin zap sliding down with a hiss under it.
    case Sfx::Fire:
        return {{{tone(Wave::Pulse12, kE7, 0.11f, kE5)}, {noise(9000.0f, 0.06f, 3000.0f, 0.3f)}}, -18.0f};
    // An alien bursts: crackling noise and a falling blip.
    case Sfx::AlienDeath:
        return {{{noise(7000.0f, 0.22f, 900.0f)}, {tone(Wave::Pulse25, kB5, 0.12f, kE4, 0.6f)}}, -13.0f};
    // The cannon: a long roar falling into a deep drop, a sad sting on top.
    case Sfx::CannonDeath:
        return {{{noise(3200.0f, 1.2f, 120.0f)},
                 {tone(Wave::Pulse25, kE5, 0.2f, kB3), tone(Wave::Pulse25, kB3, 0.4f, kE3)},
                 {tone(Wave::Triangle, kE3, 0.9f, kE2 - 7.0f)}},
                -8.0f};
    // The siren: up and down a fifth, about four times a second; looped.
    case Sfx::SaucerSiren:
        return {{{tone(Wave::Pulse25, kE5, 0.12f, kB5), tone(Wave::Pulse25, kB5, 0.12f, kE5)}}, -20.0f};
    // The mystery ship falls apart: a descending run over a roar.
    case Sfx::SaucerDeath:
        return {{{tone(Wave::Pulse25, kE6, 0.06f), tone(Wave::Pulse25, kB5, 0.06f), tone(Wave::Pulse25, kG5, 0.06f),
                  tone(Wave::Pulse25, kE5, 0.06f), tone(Wave::Pulse25, kB4, 0.06f), tone(Wave::Pulse25, kG4, 0.06f),
                  tone(Wave::Pulse25, kE4, 0.2f)},
                 {noise(4000.0f, 0.6f, 400.0f, 0.7f)}},
                -10.0f};
    case Sfx::ShieldHit: return {{{noise(2500.0f, 0.09f, 800.0f)}}, -20.0f};
    case Sfx::ShotHitBomb:
        return {{{noise(12000.0f, 0.06f, 4000.0f)}, {tone(Wave::Pulse12, kB6, 0.05f, kE6, 0.6f)}}, -17.0f};
    case Sfx::BombLanded: return {{{noise(1500.0f, 0.1f, 400.0f)}, {tone(Wave::Triangle, kE3, 0.08f, kE2)}}, -21.0f};
    case Sfx::ExtraLife:
        return {{{tone(Wave::Pulse25, kE5, 0.05f), tone(Wave::Pulse25, kG5, 0.05f), tone(Wave::Pulse25, kB5, 0.05f),
                  tone(Wave::Pulse25, kE6, 0.08f), tone(Wave::Pulse25, kE5, 0.05f), tone(Wave::Pulse25, kG5, 0.05f),
                  tone(Wave::Pulse25, kB5, 0.05f), tone(Wave::Pulse25, kE6, 0.22f)},
                 {tone(Wave::Triangle, kE3, 0.6f)}},
                -11.0f};
    case Sfx::WaveCleared:
        return {{{tone(Wave::Pulse25, kB4, 0.08f), tone(Wave::Pulse25, kE5, 0.08f), tone(Wave::Pulse25, kG5, 0.08f),
                  tone(Wave::Pulse25, kB5, 0.08f), tone(Wave::Pulse25, kE6, 0.3f)},
                 {tone(Wave::Triangle, kE3, 0.32f), tone(Wave::Triangle, kB3, 0.3f)}},
                -12.0f};
    case Sfx::WaveStart:
        return {{{tone(Wave::Pulse25, kE5, 0.07f), tone(Wave::Pulse25, kD5, 0.07f), tone(Wave::Pulse25, kB4, 0.2f)},
                 {tone(Wave::Triangle, kE3, 0.34f)}},
                -15.0f};
    case Sfx::GameOver:
        return {{{tone(Wave::Pulse50, kE5, 0.16f), tone(Wave::Pulse50, kD5, 0.16f), tone(Wave::Pulse50, kB4, 0.16f),
                  tone(Wave::Pulse50, kG4, 0.16f), tone(Wave::Pulse50, kE4, 0.6f)},
                 {tone(Wave::Triangle, kE3, 0.48f), tone(Wave::Triangle, kB3 - 12.0f, 0.3f), tone(Wave::Triangle, kE2, 0.6f)}},
                -11.0f};
    case Sfx::MenuMove: return {{{tone(Wave::Pulse12, kE6, 0.03f)}}, -18.0f};
    case Sfx::MenuSelect: return {{{tone(Wave::Pulse25, kE5, 0.04f), tone(Wave::Pulse25, kB5, 0.07f)}}, -15.0f};
    case Sfx::MenuBack: return {{{tone(Wave::Pulse25, kB5, 0.04f), tone(Wave::Pulse25, kE5, 0.07f)}}, -15.0f};
    }
    return {};
}

} // namespace

std::vector<float> renderSfx(const Sfx sfx, const std::uint32_t sampleRate)
{
    const Recipe recipe = _recipe(sfx);
    return gamelib::audio::renderMix(recipe.tracks, recipe.peakDb, sampleRate);
}

} // namespace invaders::app
