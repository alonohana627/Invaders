add_subdirectory(game)

add_executable(invaders main.cpp)
target_link_libraries(invaders PRIVATE invaders_game GameLib::app GameLib::fx)
gamelib_target_warnings(invaders)
gamelib_copy_runtime(invaders)
# The game's own procedural shapes, embedded as SPIR-V.
gamelib_add_shaders(invaders SHADERS ${PROJECT_SOURCE_DIR}/shaders/orb.frag)
