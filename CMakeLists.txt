cmake_minimum_required(VERSION 3.16)
project(sixseven LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SFML installé sur le système (Linux : paquet libsfml-dev ; Windows : vcpkg, etc.)
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

# Sinon, SFML 2.6 est téléchargé et compilé (en statique) automatiquement.
if(NOT SFML_FOUND)
    message(STATUS "SFML introuvable sur le système, téléchargement via FetchContent")
    include(FetchContent)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(SFML)
endif()

add_executable(sixseven
    src/main.cpp
    src/AI.cpp
    src/Card.cpp
    src/Deck.cpp
    src/Manager.cpp
)
target_link_libraries(sixseven PRIVATE sfml-graphics sfml-window sfml-system)

# Windows : pas de fenêtre console, tout en gardant main() comme point d'entrée
if(WIN32)
    if(MSVC)
        target_link_options(sixseven PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
    else()
        target_link_options(sixseven PRIVATE -mwindows)
    endif()
endif()

# Le dossier assets est copié à côté de l'exécutable
add_custom_command(TARGET sixseven POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:sixseven>/assets)
