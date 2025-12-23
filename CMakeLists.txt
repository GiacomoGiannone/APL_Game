cmake_minimum_required(VERSION 3.15)
project(APL_Game)

set(CMAKE_CXX_STANDARD 17)

# ============================================
# NUOVA STRUTTURA CARTELLE
# ============================================
# Root: APL_Game/
#   ├── Cpp/           (codice C++ qui dentro)
#   │   ├── src/
#   │   ├── include/
#   │   ├── net/
#   │   └── assets/
#   └── Go/           (codice Go server)
# ============================================

# Imposta il percorso base del progetto C++
set(CPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Cpp")
message(STATUS "C++ Root: ${CPP_ROOT}")

# ============================================
# SETUP SORGENTI
# ============================================
file(GLOB_RECURSE SOURCES 
    "${CPP_ROOT}/src/*.cpp"
    "${CPP_ROOT}/net/*.cpp"           
    "${CPP_ROOT}/src/**/*.cpp"        # Ricorsivo in src
    "${CPP_ROOT}/net/**/*.cpp"        # Ricorsivo in net
)

file(GLOB_RECURSE HEADERS 
    "${CPP_ROOT}/include/*.h" 
    "${CPP_ROOT}/include/*.hpp"
    "${CPP_ROOT}/net/*.h"            
    "${CPP_ROOT}/net/*.hpp"
)

message(STATUS "Trovati ${SOURCES} file sorgente")
message(STATUS "Trovati ${HEADERS} file header")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ============================================
# INCLUDE DIRECTORIES
# ============================================
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CPP_ROOT}/include"
    "${CPP_ROOT}/net"
)

# ============================================
# CONFIGURAZIONE WINDOWS
# ============================================
if(WIN32)
    set(SFML_ROOT "C:/SFML-2.5.1" CACHE PATH "Percorso di SFML")
    if(NOT EXISTS "${SFML_ROOT}/include/SFML/Graphics.hpp")
        message(FATAL_ERROR "SFML non trovato in ${SFML_ROOT}")
    endif()
    
    target_include_directories(${PROJECT_NAME} PRIVATE "${SFML_ROOT}/include")
    target_link_directories(${PROJECT_NAME} PRIVATE "${SFML_ROOT}/lib")
    target_link_libraries(${PROJECT_NAME} 
        sfml-graphics 
        sfml-window 
        sfml-system 
        sfml-audio 
        sfml-network
        opengl32 
        winmm 
        gdi32
    )

    # Copia DLL Windows
    file(GLOB SFML_DLLS "${SFML_ROOT}/bin/*.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${SFML_DLLS} 
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copia DLL SFML in output directory"
    )

# ============================================
# CONFIGURAZIONE MACOS
# ============================================
elseif(APPLE)
    # Cerchiamo SFML nella cartella 'deps/SFML' (rinominata dallo script)
    # Ora relativo alla root C++
    set(SFML_ROOT "${CPP_ROOT}/deps/SFML")
    
    message(STATUS "Configurando per macOS...")
    message(STATUS "SFML Root: ${SFML_ROOT}")

    if(NOT EXISTS "${SFML_ROOT}/include/SFML/Graphics.hpp")
        message(FATAL_ERROR "SFML non trovato in ${SFML_ROOT}. Esegui ./configure_mac.sh prima!")
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE "${SFML_ROOT}/include")
    target_link_directories(${PROJECT_NAME} PRIVATE "${SFML_ROOT}/lib")
    
    # Linkiamo le librerie .dylib
    target_link_libraries(${PROJECT_NAME} 
        "${SFML_ROOT}/lib/libsfml-graphics.dylib"
        "${SFML_ROOT}/lib/libsfml-window.dylib"
        "${SFML_ROOT}/lib/libsfml-system.dylib"
        "${SFML_ROOT}/lib/libsfml-audio.dylib"
        "${SFML_ROOT}/lib/libsfml-network.dylib" 
    )
    
    # Rpath setup
    set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "${SFML_ROOT}/lib")
    
    # Copia le dylib per sicurezza
    file(GLOB SFML_DYLIBS "${SFML_ROOT}/lib/*.dylib")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${SFML_DYLIBS} 
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copia dylib SFML in output directory"
    )
endif()

# ============================================
# ASSETS (Comune)
# ============================================
if(EXISTS "${CPP_ROOT}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CPP_ROOT}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copia assets in output directory"
    )
endif()

# ============================================
# OUTPUT CONFIGURATION
# ============================================
# Imposta l'output in una cartella bin/ nella root del progetto
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# ============================================
# TARGET PROPERTIES
# ============================================
if(APPLE)
    # Su macOS, imposta il bundle se necessario
    # set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
endif()

# Visualizza info di build
message(STATUS "==========================================")
message(STATUS "Build configurata per ${PROJECT_NAME}")
message(STATUS "Struttura:")
message(STATUS "  - Root C++: ${CPP_ROOT}")
message(STATUS "  - Sorgenti: ${SOURCES}")
message(STATUS "  - Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "==========================================")