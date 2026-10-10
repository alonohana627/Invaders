# Smoke runs: the real game in GameLib's self-test mode. Invaders has no unit
# tests (see AGENTS.md); these launches are its only CTest entries.
#
# Hidden window, validation layer REQUIRED (a missing layer fails), exit after
# N presented frames, any validation message fails, no GPU = skipped (77).
gamelib_validation_layer_dir(_layer_dir)
set(_smoke_env "INVADERS_DATA_DIR=set:${CMAKE_CURRENT_BINARY_DIR}/smoke_data")
if(_layer_dir)
    list(APPEND _smoke_env "VK_ADD_LAYER_PATH=path_list_prepend:${_layer_dir}")
endif()

function(invaders_smoke name)
    add_test(NAME invaders_smoke_${name} COMMAND invaders --hidden --require-validation ${ARGN})
    set_tests_properties(invaders_smoke_${name} PROPERTIES
        LABELS "gpu;smoke"
        TIMEOUT 180
        SKIP_RETURN_CODE 77
        ENVIRONMENT_MODIFICATION "${_smoke_env}")
endfunction()

# The title screen.
invaders_smoke(title --frames 600)
