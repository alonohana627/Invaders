#pragma once

#include <gamelib/core/color.hpp>

namespace invaders::app {

// Every color Invaders uses, authored as sRGB hex and converted to linear
// once. Glowing creatures on a dark CRT: each role owns one hue - magenta,
// cyan and violet aliens by row, green for everything on the player's side
// (cannon, shields, ground), hot red-orange for danger (bombs, the mystery
// ship), white only for flashes.
struct Palette {
    gamelib::core::Color screen;     // background, deep blue-black
    gamelib::core::Color screenGlow; // the faint phosphor haze in the middle
    gamelib::core::Color squid;
    gamelib::core::Color crab;
    gamelib::core::Color octopus;
    gamelib::core::Color cannon;
    gamelib::core::Color cannonGlow;
    gamelib::core::Color shield;
    gamelib::core::Color shieldDim;  // crumbled cells
    gamelib::core::Color ground;
    gamelib::core::Color shot;
    gamelib::core::Color bomb;
    gamelib::core::Color bombGlow;
    gamelib::core::Color saucer;
    gamelib::core::Color saucerGlow;
    gamelib::core::Color flash;
    gamelib::core::Color text;
    gamelib::core::Color textDim;
    gamelib::core::Color accent;
    gamelib::core::Color danger;
};

[[nodiscard]] inline const Palette& palette()
{
    using gamelib::core::srgb;
    static const Palette kPalette{
        .screen = srgb(0x04060F),
        .screenGlow = srgb(0x10224A),
        .squid = srgb(0xFF5AC8),
        .crab = srgb(0x3FE0FF),
        .octopus = srgb(0xA98BFF),
        .cannon = srgb(0x7DFFA8),
        .cannonGlow = srgb(0x1FD86A),
        .shield = srgb(0x3BE07A),
        .shieldDim = srgb(0x1A7A48),
        .ground = srgb(0x2BC46A),
        .shot = srgb(0xE8FFF0),
        .bomb = srgb(0xFFE2C8),
        .bombGlow = srgb(0xFF5A2E),
        .saucer = srgb(0xFF6B5A),
        .saucerGlow = srgb(0xFF2448),
        .flash = srgb(0xFFFFFF),
        .text = srgb(0xEAF6FF),
        .textDim = srgb(0x6E7CA8),
        .accent = srgb(0xFFD166),
        .danger = srgb(0xFF4F5E),
    };
    return kPalette;
}

} // namespace invaders::app
