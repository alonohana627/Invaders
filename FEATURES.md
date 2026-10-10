# Features

What Invaders does today. Only features that are implemented and verified
by running the game are listed; updated in the same commit as each feature
(see `AGENTS.md`).

## Build

- Standalone CMake project on GameLib, pinned by full commit hash from GitHub
  (`docs/GAMELIB_REVISION.md`).
- Smoke runs with the Vulkan validation layer required (`ctest --preset debug`):
  the title with its attract demo, 20 000 unthrottled frames of the demo
  gunner, and a resize plus minimize while the demo plays.

## Game

- The arcade rules: an 11 x 5 formation in three kinds (30, 20 and 10
  points) marches as one unit, drops a row on the step after touching an
  edge and speeds up as it shrinks; three kinds of bomb take turns, the
  rolling one aimed at the cannon's column; one cannon shot at a time,
  which can also hit bombs.
- Four shields made of 22 x 16 cells; shots and bombs clear and crumble the
  cells around their hit, aliens marching through eat them away.
- A mystery ship crosses the top now and then (not with fewer than 8 aliens
  left), worth 50 to 300 by the arcade's shot-count table.
- Three lives, an extra cannon at 1500 points; each wave starts lower (for
  four waves, then the depth starts over); the game ends when the lives run
  out or the aliens reach the cannon's row.
- Keyboard: arrows or A/D move, Space (or Up/W) fires; holding fire shoots
  again as soon as the last shot is gone.

## Screens

- Title menu with an attract demo (the computer gunner) behind it, pause
  menu (Escape or P), game-over menu with score and wave - GameLib's
  `ArcadeMenus`; sound volume, screen shake and fullscreen (F11) saved with
  the best score in `invaders.ini` (`ArcadeProfile`).
- `--demo`: the computer gunner plays on the playing screen, endlessly.
