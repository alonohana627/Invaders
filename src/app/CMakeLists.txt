# invaders_app: everything around the rules - the GameLib main-loop glue,
# the view, sounds and effects. Menus, the profile file, sound banks and
# effects come from GameLib (app, ui, audio, fx); what stays here is
# Invaders' content and look.
add_library(invaders_app STATIC
    invaders_game.cpp
    view.cpp)
target_include_directories(invaders_app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(invaders_app PUBLIC invaders_game GameLib::app GameLib::ui GameLib::fx GameLib::ai)
gamelib_target_warnings(invaders_app)
