cmake_minimum_required(VERSION 3.15)
project(APL_Game)

set(CMAKE_CXX_STANDARD 17)

# ============================================
# SETUP SORGENTI
# ============================================
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

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
    target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system sfml-audio opengl32 winmm gdi32)

    # Copia DLL Windows
    file(GLOB SFML_DLLS "${SFML_ROOT}/bin/*.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SFML_DLLS} $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

# ============================================
# CONFIGURAZIONE MACOS
# ============================================
elseif(APPLE)
    # Cerchiamo SFML nella cartella 'deps/SFML' (rinominata dallo script)
    set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/SFML")
    
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
    )
    
    # Rpath setup
    set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "${SFML_ROOT}/lib")
    
    # Copia le dylib per sicurezza
    file(GLOB SFML_DYLIBS "${SFML_ROOT}/lib/*.dylib")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SFML_DYLIBS} $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# ============================================
# ASSETS (Comune)
# ============================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
endif()