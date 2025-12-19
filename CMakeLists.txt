cmake_minimum_required(VERSION 3.15)
project(APL_Game)

set(CMAKE_CXX_STANDARD 17)

# ============================================
# TROVA SFML (Windows specifico)
# ============================================

set(SFML_ROOT "C:/SFML-2.5.1" CACHE PATH "Percorso di SFML")
message(STATUS "Cercando SFML in: ${SFML_ROOT}")

if(NOT EXISTS "${SFML_ROOT}/include/SFML/Graphics.hpp")
    message(FATAL_ERROR 
        "SFML non trovato in ${SFML_ROOT}!\n"
        "Scarica SFML 2.5.1 64-bit da: https://www.sfml-dev.org/download/sfml/2.5.1/\n"
        "Estrai in C:/SFML-2.5.1"
    )
endif()

set(SFML_INCLUDE_DIR "${SFML_ROOT}/include")
set(SFML_LIB_DIR "${SFML_ROOT}/lib")

# ============================================
# CONFIGURA PERCORSI DI INCLUDE
# ============================================

# I tuoi file .h in include/ vedranno SFML grazie a questo
include_directories(
    ${SFML_INCLUDE_DIR}                    # Header di SFML
    ${CMAKE_CURRENT_SOURCE_DIR}/include    # I TUOI header
)

# ============================================
# TROVA FILE SORGENTI
# ============================================

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

if(NOT SOURCES)
    message(FATAL_ERROR "Nessun file .cpp trovato in src/")
endif()

message(STATUS "Trovati ${SOURCES} file sorgenti")
message(STATUS "Trovati ${HEADERS} file header")

# ============================================
# CREA ESEGUIBILE
# ============================================

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Doppia sicurezza per gli include
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SFML_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ============================================
# LINK LIBRERIE
# ============================================

target_link_directories(${PROJECT_NAME} PRIVATE ${SFML_LIB_DIR})

target_link_libraries(${PROJECT_NAME}
    sfml-graphics
    sfml-window
    sfml-system
    opengl32.lib
    winmm.lib
    gdi32.lib
)

# ============================================
# COPIA ASSETS E DLL (Windows)
# ============================================

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copiando risorse..."
    )
endif()

# Copia DLL necessarie
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SFML_ROOT}/bin/sfml-graphics-2.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SFML_ROOT}/bin/sfml-window-2.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SFML_ROOT}/bin/sfml-system-2.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SFML_ROOT}/bin/openal32.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copiando DLL di SFML..."
)

# ============================================
# MESSAGGI FINALI
# ============================================

message(STATUS "")
message(STATUS "Configurazione completata!")
message(STATUS "Progetto: ${PROJECT_NAME}")
message(STATUS "SFML: ${SFML_ROOT}")
message(STATUS "Per compilare: cmake --build . --config Release")
message(STATUS "")