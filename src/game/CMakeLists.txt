# invaders_game: the rules - formation, shields, bombs, mystery ship, cannon -
# and the demo gunner. Depends only on GameLib::core and GameLib::ai (math,
# RNG, timers, events, interpolation, computer-player helpers): no window,
# GPU, audio or clock.
add_library(invaders_game STATIC
    formation.cpp
    gunner.cpp
    session.cpp
    shield.cpp)
target_include_directories(invaders_game PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(invaders_game PUBLIC GameLib::core GameLib::ai)
gamelib_target_warnings(invaders_game)
