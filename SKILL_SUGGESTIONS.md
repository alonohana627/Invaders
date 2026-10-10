# Skill suggestions

Skills that would have made work on this project go better. See `AGENTS.md` for
how this file is maintained.

## Already installed

In `~/.claude/skills/` (checked 2026-10-10):

- arcade-action-physics
- chiptune-sfx-synthesis
- ci-cross-platform
- cmake-build-developer
- cmake-library-consumption
- computer-graphics-developer
- cozy-sim-design
- cpp-debugging-profiling
- cpp-performance-developer
- cpp-testing
- cpp26-developer
- deterministic-replay-testing
- game-audio-developer
- game-engine-developer
- game-feel-designer
- game-library-extraction
- game-persistence
- game-ui-text-and-dialogue
- game-visual-design
- glfw-platform-developer
- glsl-shader-build-embedding
- grid-algorithms-for-games
- headless-balance-sweeps
- learning-mentor
- procedural-dungeon-generation
- puzzle-game-design-undo
- sdf-2d-shape-shaders
- shader-developer
- software-architecture-developer
- technical-docs-writer
- tilemap-and-camera-2d
- vector-arcade-neon-look
- vulkan-app-smoke-testing
- vulkan-developer
- vulkan-offscreen-postprocess
- vulkan-text-and-ui
- win32-game-window-automation
- window-screenshot-bursts
- windows-release-packaging

## Suggested

### retro-crt-screen-shader

A CRT look for a 2D game drawn as sprites on a straight-alpha renderer,
without an offscreen pass: a full-canvas background shader (dark tube,
phosphor haze, the colored cellophane bands of old cabinets) drawn first
and a full-canvas overlay drawn over the field and under the menus
(scanlines tied to framebuffer pixels so they stay crisp at any window
size, a slow refresh bar, rounded dark tube corners); composing several
translucent layers in one straight-alpha output with `over()` instead of
mixing color and alpha separately (which leaves bright blobs where a light
layer meets a dark one); keeping bands and bars faint enough that the
HUD and sprites stay readable; and checking it at two window sizes for
moire.

Why: Invaders' first CRT overlay showed white rectangles at the screen
edges (a light bar mixed into the dark corner's alpha), the cellophane
bands were far too strong and a noise grain showed as 20-pixel blocks; all
were found on screenshots and fixed by trial. `vector-arcade-neon-look`
lists CRT emulation as not covered, and `vulkan-offscreen-postprocess`
needs a render-target pass GameLib does not have.
