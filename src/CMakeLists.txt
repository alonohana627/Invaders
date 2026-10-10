add_subdirectory(game)
add_subdirectory(app)

# The executable is only main.cpp; everything else lives in the libraries.
add_executable(invaders main.cpp)
target_link_libraries(invaders PRIVATE invaders_app)
gamelib_target_warnings(invaders)
gamelib_copy_runtime(invaders)
