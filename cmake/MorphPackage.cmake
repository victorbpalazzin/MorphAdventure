# Regras de instalação e pacote ZIP (CPack). Usadas por scripts/package/.
# O pacote contém apenas executáveis e instruções; o HTML de referência
# continua no repositório e não é substituído.

if(NOT MORPH_BUILD_GAME)
    return()
endif()

set(_morph_package_targets morph-core-shell)
if(TARGET morph-adventure)
    list(APPEND _morph_package_targets morph-adventure)
endif()

install(TARGETS ${_morph_package_targets} RUNTIME DESTINATION .)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/scripts/package/LEIA-ME.txt" DESTINATION .)

if(CMAKE_SYSTEM_PROCESSOR)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _morph_arch)
else()
    set(_morph_arch "unknown")
endif()

set(CPACK_PACKAGE_NAME "MorphAdventure")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "MorphAdventure-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${_morph_arch}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")
include(CPack)
