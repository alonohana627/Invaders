# Invaders

A Space Invaders-style shooter built on
[GameLib](https://github.com/alonohana627/GameLib). Everything on screen is
drawn by shaders; every sound is synthesized.

Work in progress: the project was just generated from GameLib's new-game
template.

## Build and run

Requirements are GameLib's (C++26 Clang, CMake 3.28+, Ninja, Vulkan 1.3,
glslc, the Vulkan validation layer for the smoke runs). GameLib is fetched
from GitHub at the commit pinned in `CMakeLists.txt`.

```powershell
cmake --preset debug
cmake --build --preset debug
build/debug/bin/invaders.exe
ctest --preset debug    # smoke runs with the validation layer
```
