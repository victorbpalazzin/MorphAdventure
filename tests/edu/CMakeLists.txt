# Modo educativo (MORPH_EDUCATIONAL). Os testes rodam nos dois builds:
# o modo é escolhido explicitamente com GameInitMode, então arcade e
# educativo são verificados com a MESMA implementação ligada ao jogo.
morph_add_logic_test(edu-quiz edu_quiz_test.c)
morph_add_logic_test(edu-flow edu_flow_test.c)
if(TARGET morph::raylib)
    file(GLOB _edu_modules CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/ui/*.c" "${PROJECT_SOURCE_DIR}/src/effects/*.c"
        "${PROJECT_SOURCE_DIR}/src/items/*.c" "${PROJECT_SOURCE_DIR}/src/obstacles/*.c"
        "${PROJECT_SOURCE_DIR}/src/worlds/*.c" "${PROJECT_SOURCE_DIR}/src/player/*.c")
    list(FILTER _edu_modules EXCLUDE REGEX "combat_energy[.]c$")
    morph_add_module_test(edu-modules SOURCES ${_edu_modules} TESTS edu_modules_test.c)
    add_executable(edu-lab edu_lab.c ${_edu_modules} ${PROJECT_SOURCE_DIR}/src/core/composition.c)
    target_include_directories(edu-lab PRIVATE ${MORPH_TEST_SUPPORT_DIR})
    target_link_libraries(edu-lab PRIVATE morph_core morph::raylib)
    morph_project_target(edu-lab)
    morph_standalone_runtime(edu-lab)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/captures")
    add_test(NAME edu-flow-gui COMMAND edu-lab --capture "${CMAKE_CURRENT_BINARY_DIR}/captures")
    set_tests_properties(edu-flow-gui PROPERTIES LABELS "gui;edu;integration" TIMEOUT 180)
    if(UNIX AND NOT APPLE AND NOT DEFINED ENV{DISPLAY} AND NOT DEFINED ENV{WAYLAND_DISPLAY})
        find_program(EDU_XVFB xvfb-run)
        set_tests_properties(edu-flow-gui PROPERTIES DISABLED TRUE)
        if(EDU_XVFB)
            add_test(NAME edu-flow-gui-xvfb COMMAND ${EDU_XVFB} -a $<TARGET_FILE:edu-lab> --capture "${CMAKE_CURRENT_BINARY_DIR}/captures")
            set_tests_properties(edu-flow-gui-xvfb PROPERTIES LABELS "gui;edu;integration" TIMEOUT 180)
        endif()
    endif()
endif()
