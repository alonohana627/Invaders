# invaders_app: everything around the rules - the GameLib main-loop glue,
# the view, sounds and effects. Menus, the profile file, sound banks and
# effects come from GameLib (app, ui, audio, fx); what stays here is
# Invaders' content and look.
add_library(invaders_app STATIC
    invaders_game.cpp
    sounds.cpp
    view.cpp)
target_include_directories(invaders_app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(invaders_app PUBLIC invaders_game GameLib::app GameLib::ui GameLib::fx GameLib::ai)
gamelib_target_warnings(invaders_app)
# Invaders' own procedural shapes, embedded as SPIR-V (see view.hpp, Shapes).
gamelib_add_shaders(invaders_app SHADERS
    ${PROJECT_SOURCE_DIR}/shaders/alien.frag
    ${PROJECT_SOURCE_DIR}/shaders/bomb.frag
    ${PROJECT_SOURCE_DIR}/shaders/cannon.frag
    ${PROJECT_SOURCE_DIR}/shaders/crt.frag
    ${PROJECT_SOURCE_DIR}/shaders/saucer.frag
    ${PROJECT_SOURCE_DIR}/shaders/shield.frag)
