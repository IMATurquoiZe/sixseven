cmake_minimum_required(VERSION 3.16)
project(sixseven LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(sixseven
    src/main.cpp
    src/Card.cpp
    src/Deck.cpp
    src/Manager.cpp
)
target_link_libraries(sixseven PRIVATE sfml-graphics sfml-window sfml-system)

# Le dossier assets est copié à côté de l'exécutable
add_custom_command(TARGET sixseven POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:sixseven>/assets)
