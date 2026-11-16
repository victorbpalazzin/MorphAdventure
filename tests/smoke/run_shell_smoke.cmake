# Executa a janela de diagnóstico real por 5 quadros e confere:
#  - código de saída 0;
#  - linha "core-shell: initialized=1 ready=0 missing=0x3f frames=5"
#    (ready=0/missing=0x3f são esperados enquanto as pessoas 2–6 não entregam);
#  - screenshot PNG gerado e não vazio.
# Variáveis: EXE, WORK_DIR, LAUNCHER (lista; vazio = executar direto).
if(NOT EXE OR NOT WORK_DIR)
    message(FATAL_ERROR "Uso: cmake -DEXE=... -DWORK_DIR=... [-DLAUNCHER=...] -P run_shell_smoke.cmake")
endif()

set(_expected_ready "0")
set(_expected_missing "0x3f")
if(DEFINED EXPECT_READY)
    set(_expected_ready "${EXPECT_READY}")
endif()
if(DEFINED EXPECT_MISSING)
    set(_expected_missing "${EXPECT_MISSING}")
endif()

# LAUNCHER chega com "|" no lugar de ";" para não ser quebrado pelo add_test.
string(REPLACE "|" ";" LAUNCHER "${LAUNCHER}")

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")
set(_png "core-shell.png")

execute_process(
    COMMAND ${LAUNCHER} "${EXE}" --smoke-frames 5 --capture ${_png}
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
    TIMEOUT 90)

message(STATUS "saida:\n${_out}")
if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "morph-core-shell terminou com código ${_rc}.\nstderr:\n${_err}")
endif()
set(_line "core-shell: initialized=1 ready=${_expected_ready} missing=${_expected_missing} frames=5")
string(FIND "${_out}" "${_line}" _pos)
if(_pos EQUAL -1)
    message(FATAL_ERROR "Linha esperada não encontrada: '${_line}'\nstderr:\n${_err}")
endif()
if(NOT EXISTS "${WORK_DIR}/${_png}")
    message(FATAL_ERROR "Screenshot ${WORK_DIR}/${_png} não foi gerado.")
endif()
file(SIZE "${WORK_DIR}/${_png}" _size)
if(_size LESS 1024)
    message(FATAL_ERROR "Screenshot muito pequeno (${_size} bytes).")
endif()
message(STATUS "OK: janela abriu, 5 quadros, screenshot ${_size} bytes em ${WORK_DIR}/${_png}")
