if(TARGET morph::raylib)
    set(_item_sources ${PROJECT_SOURCE_DIR}/src/items/items.c ${PROJECT_SOURCE_DIR}/src/items/items_draw.c)
    morph_add_module_test(items-rules SOURCES ${_item_sources} TESTS items_test.c)
    morph_add_module_test(items-core SOURCES ${_item_sources} TESTS items_integration.c)
    morph_add_module_test(items-passages SOURCES ${_item_sources}
        ${PROJECT_SOURCE_DIR}/src/obstacles/obstacles.c
        ${PROJECT_SOURCE_DIR}/src/obstacles/obstacles_draw.c
        ${PROJECT_SOURCE_DIR}/src/player/collision.c
        ${PROJECT_SOURCE_DIR}/src/worlds/worlds.c
        ${PROJECT_SOURCE_DIR}/src/worlds/vector.c
        ${PROJECT_SOURCE_DIR}/src/worlds/layers.c
        ${PROJECT_SOURCE_DIR}/src/worlds/background.c
        TESTS items_passages.c)
    add_executable(items-preview items_preview.c ${_item_sources})
    target_include_directories(items-preview PRIVATE ${MORPH_TEST_SUPPORT_DIR})
    target_link_libraries(items-preview PRIVATE morph_core morph::raylib)
    morph_project_target(items-preview)
    morph_standalone_runtime(items-preview)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/captures")
    add_test(NAME items-render COMMAND items-preview "${CMAKE_CURRENT_BINARY_DIR}/captures")
    set_tests_properties(items-render PROPERTIES LABELS "gui;items" TIMEOUT 120)
    if(UNIX AND NOT APPLE AND NOT DEFINED ENV{DISPLAY} AND NOT DEFINED ENV{WAYLAND_DISPLAY})
        find_program(ITEMS_XVFB xvfb-run)
        if(ITEMS_XVFB)
            set_tests_properties(items-render PROPERTIES DISABLED TRUE)
            add_test(NAME items-render-xvfb COMMAND ${ITEMS_XVFB} -a $<TARGET_FILE:items-preview> "${CMAKE_CURRENT_BINARY_DIR}/captures")
        else()
            set_tests_properties(items-render PROPERTIES DISABLED TRUE)
        endif()
    endif()
endif()
