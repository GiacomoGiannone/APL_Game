# cmake/FindSFML.cmake - Versione semplificata

# Cerca gli header
find_path(SFML_INCLUDE_DIR SFML/Graphics.hpp
    PATHS
        /usr/include
        /usr/local/include
        /opt/homebrew/include           # macOS Apple Silicon
        /usr/local/opt/sfml/include     # macOS Homebrew
        /opt/local/include              # MacPorts
        C:/SFML-2.5.1/include
        C:/Program Files/SFML-2.5.1/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/SFML-2.5.1/include
    DOC "Percorso degli header di SFML"
)

# Cerca le librerie
set(SFML_LIB_COMPONENTS graphics window system)

foreach(COMPONENT ${SFML_LIB_COMPONENTS})
    find_library(SFML_${COMPONENT}_LIBRARY
        NAMES sfml-${COMPONENT} sfml-${COMPONENT}-2
        PATHS
            /usr/lib
            /usr/local/lib
            /opt/homebrew/lib
            /usr/local/opt/sfml/lib
            /opt/local/lib
            C:/SFML-2.5.1/lib
            C:/Program Files/SFML-2.5.1/lib
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/SFML-2.5.1/lib
        DOC "Libreria SFML ${COMPONENT}"
    )
    
    list(APPEND SFML_LIBRARIES ${SFML_${COMPONENT}_LIBRARY})
endforeach()

# Gestione del risultato
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SFML
    REQUIRED_VARS 
        SFML_INCLUDE_DIR 
        SFML_graphics_LIBRARY 
        SFML_window_LIBRARY 
        SFML_system_LIBRARY
    VERSION_VAR SFML_VERSION
)

if(SFML_FOUND)
    set(SFML_INCLUDE_DIRS ${SFML_INCLUDE_DIR})
    set(SFML_LIBRARIES ${SFML_LIBRARIES})
    
    # Crea target importati se non esistono
    if(NOT TARGET SFML::System)
        add_library(SFML::System UNKNOWN IMPORTED)
        set_target_properties(SFML::System PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}"
            IMPORTED_LOCATION "${SFML_system_LIBRARY}"
        )
    endif()
    
    if(NOT TARGET SFML::Window)
        add_library(SFML::Window UNKNOWN IMPORTED)
        set_target_properties(SFML::Window PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}"
            IMPORTED_LOCATION "${SFML_window_LIBRARY}"
        )
        target_link_libraries(SFML::Window INTERFACE SFML::System)
    endif()
    
    if(NOT TARGET SFML::Graphics)
        add_library(SFML::Graphics UNKNOWN IMPORTED)
        set_target_properties(SFML::Graphics PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}"
            IMPORTED_LOCATION "${SFML_graphics_LIBRARY}"
        )
        target_link_libraries(SFML::Graphics INTERFACE SFML::Window)
    endif()
endif()

mark_as_advanced(
    SFML_INCLUDE_DIR
    SFML_graphics_LIBRARY
    SFML_window_LIBRARY
    SFML_system_LIBRARY
)