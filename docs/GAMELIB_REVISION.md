# GameLib revision

Invaders and GameLib are separate repositories, and commits across
repositories are not atomic. This file records which GameLib revision each
Invaders change was validated against (build with zero warnings, smoke runs
with the validation layer). `CMakeLists.txt` pins GameLib by full commit hash
(`GIT_TAG`, `GIT_SHALLOW FALSE`), so checking out an Invaders commit
reproduces its combination by itself; CMake prints the revision it builds
against (`Invaders: using GameLib ...`).

| Invaders change | GameLib revision | Validated |
|---|---|---|
| Project from the new-game template (`build`) | `26a257b` | clean build fetching the commit, 1/1 smoke run |
| Rules, controls, shaders and menus (`feat`) | `26a257b` | debug build with zero warnings, 3/3 smoke runs (title, unthrottled demo, resize + minimize), zero validation messages |
| Re-pin for looping sounds (`audio::Player::loop`, GameLib `feat(audio)`), sounds and the siren (`feat`) | `520a4d5` | debug build with zero warnings, 3/3 smoke runs, zero validation messages |
