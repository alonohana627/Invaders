# Invaders

A Space Invaders-style shooter built on
[GameLib](https://github.com/alonohana627/GameLib): an 11 x 5 formation of
glowing creatures marches down a CRT screen, speeding up as it shrinks,
while you defend four eroding shields with one shot at a time. Everything
on screen is drawn by shaders (no image files); every sound is synthesized.

## Play

| Key | Action |
|---|---|
| Left / Right, A / D | move the cannon |
| Space (or Up / W) | fire (hold to fire again as soon as the shot is gone) |
| Escape or P | pause menu |
| F11 | fullscreen |
| Arrows, Enter, Escape | menus |

- Squid 30, crab 20, octopus 10 points; the mystery ship is worth 50 to 300
  (the arcade's table, chosen by how many shots you have fired).
- Three cannons, an extra one at 1500 points. Each wave starts lower. The
  game ends when the cannons run out or the aliens reach your row.
- Bombs: the rolling one is aimed at you; your shot can destroy bombs.

The title runs an attract demo; `--demo` lets the computer play the real
game endlessly. Settings (sound, screen shake, fullscreen) and the best
score live in `invaders.ini` in the user data folder (`%APPDATA%\Invaders`
on Windows; `INVADERS_DATA_DIR` moves it).

## Build and run

Requirements are GameLib's (C++26 Clang, CMake 3.28+, Ninja, Vulkan 1.3,
glslc, the Vulkan validation layer for the smoke runs). GameLib is fetched
from GitHub at the commit pinned in `CMakeLists.txt` (see
`docs/GAMELIB_REVISION.md`).

```powershell
cmake --preset debug
cmake --build --preset debug
build/debug/bin/invaders.exe
ctest --preset debug    # smoke runs with the validation layer
```

## Layout

- `src/game` - the rules (formation, shields, bombs, mystery ship, cannon)
  and the demo gunner; links only `GameLib::core` and `GameLib::ai`.
- `src/app` - GameLib main-loop glue, view, sounds and effects.
- `shaders/` - aliens, cannon, mystery ship, bombs, shield rows and the CRT.
- `smoke/` - CTest smoke runs (title, unthrottled demo, resize + minimize).

See `FEATURES.md` for everything the game does and `AGENTS.md` for the
rules of the repository.
