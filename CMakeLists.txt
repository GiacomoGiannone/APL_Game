cmake_minimum_required(VERSION 3.15)
project(IlVostroProgetto VERSION 1.0.0 LANGUAGES CXX)

# ============================================
# CONFIGURAZIONE BASE
# ============================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Directory di output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================
# TROVA SFML - APPROCCIO DIRETTO
# ============================================

message(STATUS "Sistema operativo: ${CMAKE_SYSTEM_NAME}")

if(WIN32)
    # WINDOWS - Percorsi specifici
    message(STATUS "Configurazione Windows")
    
    # Cerca SFML in percorsi comuni
    set(SFML_ROOT "" CACHE PATH "Percorso di SFML")
    
    if(NOT SFML_ROOT)
        # Prova percorsi automatici
        if(EXISTS "C:/SFML-2.5.1")
            set(SFML_ROOT "C:/SFML-2.5.1")
        elseif(EXISTS "C:/Program Files/SFML-2.5.1")
            set(SFML_ROOT "C:/Program Files/SFML-2.5.1")
        elseif(EXISTS "C:/Librerie/SFML-2.5.1")
            set(SFML_ROOT "C:/Librerie/SFML-2.5.1")
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/SFML-2.5.1")
            set(SFML_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/SFML-2.5.1")
        endif()
    endif()
    
    if(NOT SFML_ROOT)
        message(FATAL_ERROR 
            "SFML non trovato! Per favore:\n"
            "1. Scarica SFML 2.5.1 da https://www.sfml-dev.org/download/sfml/2.5.1/\n"
            "2. Estrai in C:/SFML-2.5.1\n"
            "3. Oppure specifica il percorso con -DSFML_ROOT=\"tuo/path\""
        )
    endif()
    
    message(STATUS "SFML trovato in: ${SFML_ROOT}")
    
    # Imposta percorsi
    set(SFML_INCLUDE_DIR "${SFML_ROOT}/include")
    set(SFML_LIB_DIR "${SFML_ROOT}/lib")
    
    # Verifica che i file esistano
    if(NOT EXISTS "${SFML_INCLUDE_DIR}/SFML/Graphics.hpp")
        message(FATAL_ERROR "Header di SFML non trovato in: ${SFML_INCLUDE_DIR}")
    endif()
    
    if(NOT EXISTS "${SFML_LIB_DIR}/sfml-graphics.lib")
        message(FATAL_ERROR "Librerie SFML non trovate in: ${SFML_LIB_DIR}")
    endif()
    
    # Definisci le librerie
    set(SFML_GRAPHICS_LIB "${SFML_LIB_DIR}/sfml-graphics.lib")
    set(SFML_WINDOW_LIB "${SFML_LIB_DIR}/sfml-window.lib")
    set(SFML_SYSTEM_LIB "${SFML_LIB_DIR}/sfml-system.lib")
    
    # Altre librerie necessarie su Windows
    set(SFML_EXTRA_LIBS opengl32.lib winmm.lib gdi32.lib)
    
elseif(APPLE)
    # macOS - Homebrew
    message(STATUS "Configurazione macOS")
    
    # Cerca SFML con Homebrew
    execute_process(
        COMMAND brew --prefix sfml
        OUTPUT_VARIABLE SFML_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    if(NOT SFML_ROOT OR NOT EXISTS "${SFML_ROOT}")
        # Fallback: percorsi standard di Homebrew
        if(EXISTS "/opt/homebrew/opt/sfml")  # Apple Silicon
            set(SFML_ROOT "/opt/homebrew/opt/sfml")
        elseif(EXISTS "/usr/local/opt/sfml")  # Intel
            set(SFML_ROOT "/usr/local/opt/sfml")
        else()
            message(FATAL_ERROR 
                "SFML non trovato su macOS! Per favore:\n"
                "1. Installa Homebrew: https://brew.sh\n"
                "2. Installa SFML: brew install sfml"
            )
        endif()
    endif()
    
    message(STATUS "SFML trovato in: ${SFML_ROOT}")
    
    set(SFML_INCLUDE_DIR "${SFML_ROOT}/include")
    set(SFML_LIB_DIR "${SFML_ROOT}/lib")
    
    # Cerca le librerie
    find_library(SFML_GRAPHICS_LIB sfml-graphics PATHS ${SFML_LIB_DIR})
    find_library(SFML_WINDOW_LIB sfml-window PATHS ${SFML_LIB_DIR})
    find_library(SFML_SYSTEM_LIB sfml-system PATHS ${SFML_LIB_DIR})
    
    set(SFML_EXTRA_LIBS "")
    
else()
    # Linux/Unix
    message(STATUS "Configurazione Linux/Unix")
    
    # Usa find_package standard
    find_package(SFML 2.5 COMPONENTS graphics window system QUIET)
    
    if(NOT SFML_FOUND)
        # Fallback: cerca manualmente
        find_path(SFML_INCLUDE_DIR SFML/Graphics.hpp
            PATHS /usr/include /usr/local/include
        )
        
        find_library(SFML_GRAPHICS_LIB sfml-graphics
            PATHS /usr/lib /usr/local/lib
        )
        find_library(SFML_WINDOW_LIB sfml-window
            PATHS /usr/lib /usr/local/lib
        )
        find_library(SFML_SYSTEM_LIB sfml-system
            PATHS /usr/lib /usr/local/lib
        )
        
        if(SFML_INCLUDE_DIR AND SFML_GRAPHICS_LIB AND SFML_WINDOW_LIB AND SFML_SYSTEM_LIB)
            set(SFML_FOUND TRUE)
        else()
            message(FATAL_ERROR 
                "SFML non trovato! Per favore installa:\n"
                "Ubuntu/Debian: sudo apt-get install libsfml-dev\n"
                "Fedora: sudo dnf install SFML-devel"
            )
        endif()
    else()
        # Usa le variabili trovate da find_package
        set(SFML_GRAPHICS_LIB ${SFML_GRAPHICS_LIBRARY})
        set(SFML_WINDOW_LIB ${SFML_WINDOW_LIBRARY})
        set(SFML_SYSTEM_LIB ${SFML_SYSTEM_LIBRARY})
    endif()
    
    set(SFML_EXTRA_LIBS pthread GL X11)
endif()

# ============================================
# CREA L'ESEGUIBILE
# ============================================

# Trova file sorgenti
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h")

if(NOT SOURCES)
    message(WARNING "Nessun file sorgente trovato. Creo main.cpp di esempio...")
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
        "#include <SFML/Graphics.hpp>\n\n"
        "int main() {\n"
        "    sf::RenderWindow window(sf::VideoMode(800, 600), \"SFML Works!\");\n"
        "    sf::CircleShape circle(100.f);\n"
        "    circle.setFillColor(sf::Color::Green);\n"
        "    circle.setPosition(300.f, 200.f);\n\n"
        "    while (window.isOpen()) {\n"
        "        sf::Event event;\n"
        "        while (window.pollEvent(event)) {\n"
        "            if (event.type == sf::Event::Closed)\n"
        "                window.close();\n"
        "        }\n\n"
        "        window.clear(sf::Color::Black);\n"
        "        window.draw(circle);\n"
        "        window.display();\n"
        "    }\n\n"
        "    return 0;\n"
        "}\n"
    )
    set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ============================================
# CONFIGURA IL LINK
# ============================================

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SFML_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link directories (solo Windows)
if(WIN32)
    target_link_directories(${PROJECT_NAME} PRIVATE ${SFML_LIB_DIR})
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${SFML_GRAPHICS_LIB}
    ${SFML_WINDOW_LIB}
    ${SFML_SYSTEM_LIB}
    ${SFML_EXTRA_LIBS}
)

# ============================================
# COPIA RISORSE E DLL (Windows)
# ============================================

# Copia assets
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copiando risorse..."
    )
endif()

# Copia DLL su Windows
if(WIN32)
    # Trova le DLL
    set(SFML_DLL_DIR "${SFML_ROOT}/bin")
    
    if(EXISTS "${SFML_DLL_DIR}/sfml-graphics-2.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DLL_DIR}/sfml-graphics-2.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DLL_DIR}/sfml-window-2.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DLL_DIR}/sfml-system-2.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DLL_DIR}/openal32.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copiando DLL di SFML..."
        )
    else()
        message(WARNING "DLL di SFML non trovate in ${SFML_DLL_DIR}")
    endif()
endif()

# ============================================
# MESSAGGI FINALI
# ============================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Configurazione completata!")
message(STATUS "Progetto: ${PROJECT_NAME}")
message(STATUS "Sistema: ${CMAKE_SYSTEM_NAME}")
message(STATUS "SFML: ${SFML_ROOT}")
if(WIN32)
    message(STATUS "Compiler: Visual Studio")
    message(STATUS "Per compilare: cmake --build . --config Release")
else()
    message(STATUS "Per compilare: make")
endif()
message(STATUS "========================================")