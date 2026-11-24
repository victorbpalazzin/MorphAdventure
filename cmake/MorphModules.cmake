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
foreach(_dir player worlds obstacles items ui effects audio)
    file(GLOB _found CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/${_dir}/*.c")
    if(NOT _found)
        message(FATAL_ERROR "MORPH_MODULES_LINKED=ON, mas src/${_dir}/ não tem arquivos .c. "
                            "O módulo ainda não foi entregue.")
    endif()
    list(FILTER _found EXCLUDE REGEX "(combat_energy|audio_cues)[.]c$")
    list(APPEND _morph_module_sources ${_found})
endforeach()

add_executable(morph-adventure src/main.c "${_morph_composition}" ${_morph_module_sources})
target_compile_definitions(morph-adventure PRIVATE MORPH_MODULES_LINKED)
target_link_libraries(morph-adventure PRIVATE morph_core morph::raylib)
morph_project_target(morph-adventure)
morph_standalone_runtime(morph-adventure)

# Recursos de áudio ao lado do executável: o jogo procura <pasta do exe>/assets/audio,
# então funciona igual no build de desenvolvimento e no pacote extraído (sem caminhos absolutos).
set(_morph_audio_assets "${CMAKE_CURRENT_SOURCE_DIR}/assets/audio")
if(EXISTS "${_morph_audio_assets}")
    # Pasta do executável sem depender do alvo (evita ciclo): raiz do build, ou
    # build/<Config> nos geradores multi-configuração (Visual Studio).
    if(_morph_multi_config)
        set(_morph_exe_dir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
    else()
        set(_morph_exe_dir "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    # Alvo sempre executado: arquivos de áudio atualizados chegam ao build sem recompilar o jogo.
    add_custom_target(morph-audio-assets ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_morph_exe_dir}/assets/audio"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${_morph_audio_assets}/music" "${_morph_exe_dir}/assets/audio/music"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${_morph_audio_assets}/sfx" "${_morph_exe_dir}/assets/audio/sfx"
        COMMENT "Sincronizando assets/audio junto do executavel")
    add_dependencies(morph-adventure morph-audio-assets)
else()
    message(WARNING "assets/audio nao encontrado: o jogo rodara em silencio.")
endif()
