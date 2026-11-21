# Morph Adventure — build compartilhado (pessoa 7).
# C99 + raylib 5.5 fixada. Ver docs/BUILD.md e docs/TESTING.md.
cmake_minimum_required(VERSION 3.16)

project(MorphAdventure VERSION 0.1.0 LANGUAGES C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "Build dentro da pasta do código não é permitido. Use uma pasta separada, por exemplo:\n"
        "  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release")
endif()

option(MORPH_BUILD_GAME "Compila os executáveis que dependem de raylib" ON)
option(MORPH_BUILD_TESTS "Compila os testes de lógica sem janela (CTest)" ON)
option(MORPH_WARNINGS_AS_ERRORS "Trata warnings do código do projeto como erro" ON)
option(MORPH_MODULES_LINKED "Liga os módulos reais (exige src/core/composition.c da pessoa 1)" OFF)
set(MORPH_SANITIZE "" CACHE STRING
    "Sanitizers para GCC/Clang, separados por vírgula (ex.: address,undefined). Vazio desliga.")

get_property(_morph_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _morph_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug, Release, RelWithDebInfo ou MinSizeRel" FORCE)
    message(STATUS "CMAKE_BUILD_TYPE não informado: usando Release")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/MorphWarnings.cmake)

# ---------------------------------------------------------------------------
# Núcleo da pessoa 1: puro C, sem raylib. Usado pelo jogo e pelos testes.
# ---------------------------------------------------------------------------
add_library(morph_core STATIC
    src/player/collision.c
    src/boss/boss.c
    src/items/combat_energy.c
    src/core/game.c
    src/core/viewport.c)
target_include_directories(morph_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(NOT WIN32)
    target_link_libraries(morph_core PUBLIC m)
endif()
morph_project_target(morph_core)

# ---------------------------------------------------------------------------
# Executáveis gráficos.
# ---------------------------------------------------------------------------
if(MORPH_BUILD_GAME)
    include(cmake/MorphRaylib.cmake)

    # Janela de diagnóstico real: mostra os módulos pendentes. Não é o jogo.
    add_executable(morph-core-shell src/main.c)
    target_link_libraries(morph-core-shell PRIVATE morph_core morph::raylib)
    morph_project_target(morph-core-shell)
    morph_standalone_runtime(morph-core-shell)

    if(MORPH_MODULES_LINKED)
        include(cmake/MorphModules.cmake)
    endif()
elseif(MORPH_MODULES_LINKED)
    message(FATAL_ERROR "MORPH_MODULES_LINKED=ON exige MORPH_BUILD_GAME=ON.")
endif()

# ---------------------------------------------------------------------------
# Testes.
# ---------------------------------------------------------------------------
if(MORPH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include(cmake/MorphPackage.cmake)

message(STATUS "Morph Adventure ${PROJECT_VERSION}: jogo=${MORPH_BUILD_GAME} testes=${MORPH_BUILD_TESTS} "
               "modulos_ligados=${MORPH_MODULES_LINKED} sanitize='${MORPH_SANITIZE}'")
