# Sistema de áudio. audio-cues: lógica pura. audio-backend: raylib real (dispositivo
# opcional: sem placa de som, verifica o modo silencioso). audio-flow: jogo completo.
morph_add_logic_test(audio-cues audio_cues_test.c)
if(TARGET morph::raylib)
    morph_add_module_test(audio-backend SOURCES ${PROJECT_SOURCE_DIR}/src/audio/audio.c TESTS audio_backend_test.c)
    # Cópia com um efeito e uma música removidos: prova o tratamento de arquivos ausentes.
    set(_broken "${CMAKE_CURRENT_BINARY_DIR}/assets-incompletos")
    file(REMOVE_RECURSE "${_broken}")
    file(COPY "${PROJECT_SOURCE_DIR}/assets/audio/music" "${PROJECT_SOURCE_DIR}/assets/audio/sfx" DESTINATION "${_broken}")
    file(GLOB _beam "${_broken}/sfx/beam.*")
    file(REMOVE ${_beam} "${_broken}/music/ar.ogg")
    # O teste registrado pela função não recebe argumentos; esta versão recebe a
    # pasta completa e a pasta com arquivos faltando.
    add_test(NAME audio-backend-run COMMAND audio-backend "${PROJECT_SOURCE_DIR}/assets/audio" "${_broken}")
    set_tests_properties(audio-backend-run PROPERTIES LABELS "module;audio" TIMEOUT 120 WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    set_tests_properties(audio-backend PROPERTIES DISABLED TRUE)
endif()
if(TARGET morph::raylib AND MORPH_MODULES_LINKED)
    file(GLOB _audio_flow_sources CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/ui/*.c" "${PROJECT_SOURCE_DIR}/src/effects/*.c" "${PROJECT_SOURCE_DIR}/src/items/*.c"
        "${PROJECT_SOURCE_DIR}/src/obstacles/*.c" "${PROJECT_SOURCE_DIR}/src/worlds/*.c" "${PROJECT_SOURCE_DIR}/src/player/*.c"
        "${PROJECT_SOURCE_DIR}/src/audio/*.c")
    list(FILTER _audio_flow_sources EXCLUDE REGEX "(combat_energy|audio_cues)[.]c$")
    add_executable(audio-flow audio_flow.c ${_audio_flow_sources} ${PROJECT_SOURCE_DIR}/src/core/composition.c)
    target_include_directories(audio-flow PRIVATE ${MORPH_TEST_SUPPORT_DIR})
    target_link_libraries(audio-flow PRIVATE morph_core morph::raylib)
    morph_project_target(audio-flow)
    set(_flow_cmd audio-flow "${PROJECT_SOURCE_DIR}/assets/audio")
    if(UNIX AND NOT APPLE AND NOT DEFINED ENV{DISPLAY} AND NOT DEFINED ENV{WAYLAND_DISPLAY})
        find_program(AUDIO_XVFB xvfb-run)
        if(AUDIO_XVFB)
            set(_flow_cmd ${AUDIO_XVFB} -a $<TARGET_FILE:audio-flow> "${PROJECT_SOURCE_DIR}/assets/audio")
        endif()
    endif()
    add_test(NAME audio-flow COMMAND ${_flow_cmd})
    set_tests_properties(audio-flow PROPERTIES LABELS "gui;audio;integration" TIMEOUT 240 WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()
