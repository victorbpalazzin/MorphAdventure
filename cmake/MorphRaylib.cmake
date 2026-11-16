# Fornece o alvo morph::raylib com raylib 5.5 EXATA.
#
# MORPH_RAYLIB_PROVIDER:
#   FETCH    (padrão) baixa o código oficial na primeira configuração (tag 5.5,
#            commit c1ab645ca298a2801097931d1079b10ff7eb9df8) e compila junto.
#            Offline: -DFETCHCONTENT_SOURCE_DIR_RAYLIB=<pasta com o código 5.5>.
#   PREBUILT usa um pacote binário oficial já extraído (ex.:
#            raylib-5.5_win64_mingw-w64), apontado por MORPH_RAYLIB_DIR.
#   SYSTEM   usa uma raylib instalada no sistema (find_package).
# Nenhum modo instala ferramentas no sistema.

set(MORPH_RAYLIB_VERSION "5.5")
set(MORPH_RAYLIB_COMMIT "c1ab645ca298a2801097931d1079b10ff7eb9df8")
set(MORPH_RAYLIB_PROVIDER "FETCH" CACHE STRING "Origem da raylib 5.5: FETCH, PREBUILT ou SYSTEM")
set_property(CACHE MORPH_RAYLIB_PROVIDER PROPERTY STRINGS FETCH PREBUILT SYSTEM)
set(MORPH_RAYLIB_DIR "" CACHE PATH "Pasta da raylib 5.5 pré-compilada (contém include/ e lib/)")

# Confere a versão pelo cabeçalho realmente usado, em qualquer modo.
function(morph_check_raylib_header header)
    if(NOT EXISTS "${header}")
        message(FATAL_ERROR "raylib.h não encontrado em ${header}.")
    endif()
    file(STRINGS "${header}" _major REGEX "^#define RAYLIB_VERSION_MAJOR +[0-9]+")
    file(STRINGS "${header}" _minor REGEX "^#define RAYLIB_VERSION_MINOR +[0-9]+")
    string(REGEX MATCH "[0-9]+$" _major "${_major}")
    string(REGEX MATCH "[0-9]+$" _minor "${_minor}")
    if(NOT "${_major}.${_minor}" STREQUAL "${MORPH_RAYLIB_VERSION}")
        message(FATAL_ERROR
            "O projeto fixa raylib ${MORPH_RAYLIB_VERSION}, mas ${header} é ${_major}.${_minor}.\n"
            "Use a versão 5.5 ou combine a mudança de versão com a equipe (pessoas 1 e 7).")
    endif()
    message(STATUS "raylib ${_major}.${_minor} confirmada em ${header}")
endfunction()

if(MORPH_RAYLIB_PROVIDER STREQUAL "FETCH")
    include(FetchContent)
    # Opções da raylib: só a biblioteca estática, sem exemplos.
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(PLATFORM "Desktop" CACHE STRING "" FORCE)
    FetchContent_Declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        ${MORPH_RAYLIB_COMMIT} # tag 5.5
        GIT_PROGRESS   ON)
    message(STATUS "Obtendo raylib ${MORPH_RAYLIB_VERSION} (primeira configuração pode demorar)...")
    FetchContent_MakeAvailable(raylib)
    if(NOT TARGET raylib)
        message(FATAL_ERROR
            "Falha ao obter a raylib ${MORPH_RAYLIB_VERSION}. Verifique a internet/Git ou use:\n"
            "  -DFETCHCONTENT_SOURCE_DIR_RAYLIB=<código 5.5>  (offline)\n"
            "  -DMORPH_RAYLIB_PROVIDER=PREBUILT -DMORPH_RAYLIB_DIR=<raylib-5.5_win64_mingw-w64>")
    endif()
    morph_check_raylib_header("${raylib_SOURCE_DIR}/src/raylib.h")
    set(_morph_raylib_target raylib)

elseif(MORPH_RAYLIB_PROVIDER STREQUAL "PREBUILT")
    if(NOT MORPH_RAYLIB_DIR)
        message(FATAL_ERROR
            "MORPH_RAYLIB_PROVIDER=PREBUILT exige -DMORPH_RAYLIB_DIR=<pasta>.\n"
            "Baixe o pacote oficial de https://github.com/raysan5/raylib/releases/tag/5.5\n"
            "(no Windows com MinGW: raylib-5.5_win64_mingw-w64.zip; com MSVC: raylib-5.5_win64_msvc16.zip),\n"
            "extraia e informe a pasta que contém include/ e lib/.")
    endif()
    morph_check_raylib_header("${MORPH_RAYLIB_DIR}/include/raylib.h")
    find_library(MORPH_RAYLIB_LIBRARY NAMES raylib raylibdll libraylib
        PATHS "${MORPH_RAYLIB_DIR}/lib" NO_DEFAULT_PATH)
    if(NOT MORPH_RAYLIB_LIBRARY)
        message(FATAL_ERROR "Biblioteca da raylib não encontrada em ${MORPH_RAYLIB_DIR}/lib. "
                            "Confira se o pacote baixado corresponde ao seu compilador (MinGW x MSVC).")
    endif()
    add_library(morph_raylib_prebuilt UNKNOWN IMPORTED)
    set_target_properties(morph_raylib_prebuilt PROPERTIES
        IMPORTED_LOCATION "${MORPH_RAYLIB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${MORPH_RAYLIB_DIR}/include")
    if(WIN32)
        set_property(TARGET morph_raylib_prebuilt PROPERTY INTERFACE_LINK_LIBRARIES opengl32 gdi32 winmm)
    elseif(APPLE)
        set_property(TARGET morph_raylib_prebuilt PROPERTY INTERFACE_LINK_LIBRARIES
            "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    else()
        find_package(OpenGL REQUIRED)
        find_package(Threads REQUIRED)
        set_property(TARGET morph_raylib_prebuilt PROPERTY INTERFACE_LINK_LIBRARIES
            OpenGL::GL Threads::Threads m dl rt X11)
    endif()
    set(_morph_raylib_target morph_raylib_prebuilt)

elseif(MORPH_RAYLIB_PROVIDER STREQUAL "SYSTEM")
    find_package(raylib ${MORPH_RAYLIB_VERSION} EXACT CONFIG QUIET)
    if(NOT raylib_FOUND)
        message(FATAL_ERROR
            "raylib ${MORPH_RAYLIB_VERSION} não encontrada no sistema. Instale a 5.5 ou use o padrão "
            "-DMORPH_RAYLIB_PROVIDER=FETCH.")
    endif()
    set(_morph_raylib_target raylib)

else()
    message(FATAL_ERROR "MORPH_RAYLIB_PROVIDER inválido: '${MORPH_RAYLIB_PROVIDER}'. Use FETCH, PREBUILT ou SYSTEM.")
endif()

# Alvo único consumido pelo projeto, qualquer que seja a origem.
add_library(morph_raylib INTERFACE)
target_link_libraries(morph_raylib INTERFACE ${_morph_raylib_target})
add_library(morph::raylib ALIAS morph_raylib)
