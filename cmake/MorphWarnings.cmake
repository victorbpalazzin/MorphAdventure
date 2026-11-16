# Flags de warning e sanitizers aplicados APENAS aos alvos do projeto
# (nunca à raylib, que tem warnings próprios fora do nosso controle).

function(morph_project_target target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(MORPH_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
        # snprintf/strcpy padrão C: sem avisos de "funções inseguras" do MSVC.
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(MORPH_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()

    if(MORPH_SANITIZE)
        if(MSVC)
            message(FATAL_ERROR "MORPH_SANITIZE só é suportado com GCC/Clang.")
        endif()
        string(REPLACE " " "" _sanitize "${MORPH_SANITIZE}")
        target_compile_options(${target} PRIVATE -fsanitize=${_sanitize} -fno-omit-frame-pointer -fno-sanitize-recover=all)
        target_link_options(${target} PRIVATE -fsanitize=${_sanitize})
    endif()
endfunction()

# O pacote precisa abrir numa pasta limpa: no MinGW, evita depender de
# libgcc/libwinpthread DLLs que só existem no MSYS2 do desenvolvedor.
function(morph_standalone_runtime target)
    if(MINGW)
        target_link_options(${target} PRIVATE -static -static-libgcc)
    endif()
endfunction()
