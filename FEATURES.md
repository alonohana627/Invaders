# Features

What Invaders does today. Only features that are implemented and verified
by running the game are listed; updated in the same commit as each feature
(see `AGENTS.md`).

## Build

- Standalone CMake project on GameLib, pinned by full commit hash from GitHub
  (`docs/GAMELIB_REVISION.md`).
- A smoke run with the Vulkan validation layer required (`ctest --preset debug`).
