# Executável integrado (campanha real). Só é configurado com
# -DMORPH_MODULES_LINKED=ON, depois que a pessoa 1 escrever
# src/core/composition.c com as factories reais das pessoas 2–6.
# Não existem implementações vazias aqui: faltando algo, a configuração falha.

set(_morph_composition "${CMAKE_CURRENT_SOURCE_DIR}/src/core/composition.c")
if(NOT EXISTS "${_morph_composition}")
    message(FATAL_ERROR
        "MORPH_MODULES_LINKED=ON, mas src/core/composition.c (pessoa 1) ainda não existe.\n"
        "Use MORPH_MODULES_LINKED=OFF para compilar a janela de diagnóstico.")
endif()

set(_morph_module_sources "")
foreach(_dir player worlds obstacles items ui effects)
    file(GLOB _found CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/${_dir}/*.c")
    if(NOT _found)
        message(FATAL_ERROR "MORPH_MODULES_LINKED=ON, mas src/${_dir}/ não tem arquivos .c. "
                            "O módulo ainda não foi entregue.")
    endif()
    list(FILTER _found EXCLUDE REGEX "combat_energy[.]c$")
    list(APPEND _morph_module_sources ${_found})
endforeach()

add_executable(morph-adventure src/main.c "${_morph_composition}" ${_morph_module_sources})
target_compile_definitions(morph-adventure PRIVATE MORPH_MODULES_LINKED)
target_link_libraries(morph-adventure PRIVATE morph_core morph::raylib)
morph_project_target(morph-adventure)
morph_standalone_runtime(morph-adventure)
