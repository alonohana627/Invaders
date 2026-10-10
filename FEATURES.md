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

## Look

- Every visual is a shader: glowing two-frame squid, crab and octopus
  creatures from signed distances (`shaders/alien.frag`, type and frame in
  the sprite's params; they flip frames on every march step), an SDF
  cannon, a mystery ship with chasing lights, three animated bomb kinds
  (twisting, pumping, wriggling), and shields drawn one row per sprite with
  each cell's strength in two bits, so erosion shows cell by cell.
- A procedural CRT: a dark tube with a phosphor haze and the arcade's red
  and green cellophane bands, then scanlines, a slow refresh bar and dark
  rounded corners over the field.
- HUD in glowing seven-segment digits: score, best and wave; the lives and
  spare cannons under the ground line.

## Sound

- Every sound synthesized at startup (GameLib's synth and `SoundBank`): the
  four-note marching bass (A G F E), one note per step, so it speeds up with
  the formation; the shot, alien and cannon explosions, shield crunches,
  shot-meets-bomb pops, bomb thuds, the mystery ship's death run, extra
  life, wave cleared and started, game over and menu blips.
- The mystery ship's siren loops (GameLib's `Player::loop`) for as long as
  it flies, a little louder and higher in the middle of its run; it fades
  out when the ship goes or the game pauses.
