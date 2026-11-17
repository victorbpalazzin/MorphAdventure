if(TARGET morph::raylib)
    add_executable(worlds-preview
        worlds_preview.c cpu_timer.c
        ${PROJECT_SOURCE_DIR}/src/worlds/worlds.c
        ${PROJECT_SOURCE_DIR}/src/worlds/vector.c
        ${PROJECT_SOURCE_DIR}/src/worlds/layers.c
        ${PROJECT_SOURCE_DIR}/src/worlds/background.c
        ${PROJECT_SOURCE_DIR}/src/player/player.c
        ${PROJECT_SOURCE_DIR}/src/player/player_draw.c)
    target_include_directories(worlds-preview PRIVATE ${MORPH_TEST_SUPPORT_DIR})
    target_link_libraries(worlds-preview PRIVATE morph_core morph::raylib)
    morph_project_target(worlds-preview)
    morph_standalone_runtime(worlds-preview)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/captures")
    if(UNIX AND NOT APPLE AND NOT DEFINED ENV{DISPLAY} AND NOT DEFINED ENV{WAYLAND_DISPLAY})
        find_program(WORLDS_XVFB xvfb-run)
        if(WORLDS_XVFB)
            add_test(NAME worlds-render COMMAND ${WORLDS_XVFB} -a $<TARGET_FILE:worlds-preview>
                --validate "${CMAKE_CURRENT_BINARY_DIR}/captures")
        else()
            add_test(NAME worlds-render COMMAND worlds-preview --validate "${CMAKE_CURRENT_BINARY_DIR}/captures")
            set_tests_properties(worlds-render PROPERTIES DISABLED TRUE)
        endif()
    else()
        add_test(NAME worlds-render COMMAND worlds-preview --validate "${CMAKE_CURRENT_BINARY_DIR}/captures")
    endif()
    set_tests_properties(worlds-render PROPERTIES LABELS "gui;worlds" TIMEOUT 120)
endif()
