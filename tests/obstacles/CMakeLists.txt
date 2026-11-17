if(TARGET morph::raylib)
    set(_obstacles_sources
        ${PROJECT_SOURCE_DIR}/src/obstacles/obstacles.c
        ${PROJECT_SOURCE_DIR}/src/obstacles/obstacles_draw.c
        ${PROJECT_SOURCE_DIR}/src/player/collision.c
        ${PROJECT_SOURCE_DIR}/src/player/player.c
        ${PROJECT_SOURCE_DIR}/src/player/player_draw.c
        ${PROJECT_SOURCE_DIR}/src/worlds/worlds.c
        ${PROJECT_SOURCE_DIR}/src/worlds/vector.c
        ${PROJECT_SOURCE_DIR}/src/worlds/layers.c
        ${PROJECT_SOURCE_DIR}/src/worlds/background.c)
    morph_add_module_test(obstacles-rules SOURCES ${_obstacles_sources} TESTS obstacles_test.c)
    add_executable(obstacles-preview obstacles_preview.c ${_obstacles_sources})
    target_include_directories(obstacles-preview PRIVATE ${MORPH_TEST_SUPPORT_DIR})
    target_link_libraries(obstacles-preview PRIVATE morph_core morph::raylib)
    morph_project_target(obstacles-preview)
    morph_standalone_runtime(obstacles-preview)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/captures")
    if(UNIX AND NOT APPLE AND NOT DEFINED ENV{DISPLAY} AND NOT DEFINED ENV{WAYLAND_DISPLAY})
        find_program(OBSTACLES_XVFB xvfb-run)
        if(OBSTACLES_XVFB)
            add_test(NAME obstacles-render COMMAND ${OBSTACLES_XVFB} -a $<TARGET_FILE:obstacles-preview> --capture "${CMAKE_CURRENT_BINARY_DIR}/captures")
        else()
            add_test(NAME obstacles-render COMMAND obstacles-preview --capture "${CMAKE_CURRENT_BINARY_DIR}/captures")
            set_tests_properties(obstacles-render PROPERTIES DISABLED TRUE)
        endif()
    else()
        add_test(NAME obstacles-render COMMAND obstacles-preview --capture "${CMAKE_CURRENT_BINARY_DIR}/captures")
    endif()
    set_tests_properties(obstacles-render PROPERTIES LABELS "gui;obstacles" TIMEOUT 120)
endif()
