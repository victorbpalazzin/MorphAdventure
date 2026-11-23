# Testes do Morph Adventure (pessoa 7). Rode com:
#   ctest --test-dir build --output-on-failure            (single-config)
#   ctest --test-dir build -C Release --output-on-failure (Visual Studio)
# Labels: core (lógica sem janela), contract (cabeçalhos/regras de teste), gui (janela real).

set(MORPH_TEST_SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/support")

function(morph_add_logic_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE morph_core)
    target_include_directories(${name} PRIVATE ${MORPH_TEST_SUPPORT_DIR})
    morph_project_target(${name})
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES LABELS core TIMEOUT 60)
endfunction()

# Teste de um módulo real (pessoas 2–6), exercitando as funções de produção.
# Uso em tests/<modulo>/CMakeLists.txt:
#   morph_add_module_test(player-physics
#       SOURCES ${PROJECT_SOURCE_DIR}/src/player/player.c ${PROJECT_SOURCE_DIR}/src/player/collision.c
#       TESTS   physics_test.c)
# Módulos que usam raylib para desenhar são ligados a ela, sem abrir janela.
function(morph_add_module_test name)
    cmake_parse_arguments(T "" "" "SOURCES;TESTS" ${ARGN})
    if(NOT T_SOURCES OR NOT T_TESTS)
        message(FATAL_ERROR "morph_add_module_test(${name}): informe SOURCES (produção) e TESTS.")
    endif()
    add_executable(${name} ${T_TESTS} ${T_SOURCES})
    target_link_libraries(${name} PRIVATE morph_core)
    if(TARGET morph::raylib)
        target_link_libraries(${name} PRIVATE morph::raylib)
    endif()
    target_include_directories(${name} PRIVATE ${MORPH_TEST_SUPPORT_DIR})
    morph_project_target(${name})
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES LABELS module TIMEOUT 120)
endfunction()

# Coordenador com módulos simulados SOMENTE neste teste (vindo da pessoa 1).
morph_add_logic_test(core-check core/core_check.c)
# Regras puras do núcleo e viewport, sem simulações.
morph_add_logic_test(core-rules core/core_rules_test.c)

# MORPH_CHECK precisa falhar mesmo com NDEBUG (prova para builds Release).
add_executable(check-macro-selftest support/check_macro_selftest.c)
target_include_directories(check-macro-selftest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/support)
target_compile_definitions(check-macro-selftest PRIVATE NDEBUG)
morph_project_target(check-macro-selftest)
add_test(NAME check-macro-ndebug COMMAND check-macro-selftest)
set_tests_properties(check-macro-ndebug PROPERTIES WILL_FAIL TRUE LABELS contract TIMEOUT 30)

# Cada cabeçalho do contrato compila sozinho em C99 (sem depender de ordem de include).
set(_morph_headers core/game_types.h core/module_api.h core/game.h core/viewport.h)
set(_morph_header_sources "")
foreach(_h ${_morph_headers})
    string(MAKE_C_IDENTIFIER "${_h}" _id)
    set(_src "${CMAKE_CURRENT_BINARY_DIR}/headers/${_id}.c")
    file(GENERATE OUTPUT "${_src}" CONTENT "#include \"${_h}\"\n#include \"${_h}\" /* include guard */\nint morph_header_${_id}(void) { return 0; }\n")
    list(APPEND _morph_header_sources "${_src}")
endforeach()
add_library(morph-headers-standalone OBJECT ${_morph_header_sources})
target_include_directories(morph-headers-standalone PRIVATE ${PROJECT_SOURCE_DIR}/src)
morph_project_target(morph-headers-standalone)

# Proibir assert() nos testes: ele desaparece em Release.
add_test(NAME tests-no-assert
    COMMAND ${CMAKE_COMMAND} -DTEST_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/support/no_assert.cmake)
set_tests_properties(tests-no-assert PROPERTIES LABELS contract TIMEOUT 30)

# Janela real (raylib): 5 quadros, captura de tela e mensagem de diagnóstico.
if(MORPH_BUILD_GAME)
    set(_launcher "")
    if(UNIX AND NOT APPLE)
        find_program(MORPH_XVFB_RUN xvfb-run)
        if(DEFINED ENV{DISPLAY} OR DEFINED ENV{WAYLAND_DISPLAY})
            set(_launcher "")
        elseif(MORPH_XVFB_RUN)
            set(_launcher "${MORPH_XVFB_RUN}|-a|-s|-screen 0 1280x800x24")
        else()
            message(STATUS "Sem DISPLAY e sem xvfb-run: o teste gui core-shell-smoke ficará desabilitado.")
            set(_launcher "DISABLED")
        endif()
    endif()
    add_test(NAME core-shell-smoke
        COMMAND ${CMAKE_COMMAND}
            "-DEXE=$<TARGET_FILE:morph-core-shell>"
            "-DLAUNCHER=${_launcher}"
            "-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/smoke"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/smoke/run_shell_smoke.cmake)
    set_tests_properties(core-shell-smoke PROPERTIES LABELS gui TIMEOUT 120)
    if(_launcher STREQUAL "DISABLED")
        set_tests_properties(core-shell-smoke PROPERTIES DISABLED TRUE)
    endif()
endif()

# Testes dos módulos, quando cada responsável entregar tests/<modulo>/CMakeLists.txt.
foreach(_module player worlds obstacles items ui effects integration boss edu input)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_module}/CMakeLists.txt")
        add_subdirectory(${_module})
    endif()
endforeach()
