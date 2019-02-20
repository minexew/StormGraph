NOTE: This is known not to work due to incompatible engine branch

cmake_minimum_required(VERSION 3.1)
project(tolcl)

set(CMAKE_CXX_STANDARD 14)

set(DIST_DIR ${CMAKE_SOURCE_DIR}/dist)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_subdirectory(../StormGraphLegacy    ${CMAKE_BINARY_DIR}/build-StormGraphLegacy)
add_subdirectory(../Gui             ${CMAKE_BINARY_DIR}/build-Gui)
add_subdirectory(../OpenGlDriver    ${CMAKE_BINARY_DIR}/build-OpenGLDriver)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DIST_DIR}/${PROJECT_NAME}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR}/${PROJECT_NAME}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(sources
        ${PROJECT_SOURCE_DIR}/include/Messages.hpp

        ${PROJECT_SOURCE_DIR}/src/ClientUI.cpp
        ${PROJECT_SOURCE_DIR}/src/ClientUI.hpp
        ${PROJECT_SOURCE_DIR}/src/GameClient.hpp
        ${PROJECT_SOURCE_DIR}/src/GameScene.cpp
        ${PROJECT_SOURCE_DIR}/src/GameScene.hpp
        ${PROJECT_SOURCE_DIR}/src/GameUI.cpp
        ${PROJECT_SOURCE_DIR}/src/Inventory.cpp
        ${PROJECT_SOURCE_DIR}/src/Inventory.hpp
        ${PROJECT_SOURCE_DIR}/src/ItemIconManager.cpp
        ${PROJECT_SOURCE_DIR}/src/main.cpp
        ${PROJECT_SOURCE_DIR}/src/MainMenu.cpp
        ${PROJECT_SOURCE_DIR}/src/MainMenu.hpp
        ${PROJECT_SOURCE_DIR}/src/Map.cpp
        ${PROJECT_SOURCE_DIR}/src/Map.hpp
        ${PROJECT_SOURCE_DIR}/src/Player.cpp
        ${PROJECT_SOURCE_DIR}/src/Player.hpp
        ${PROJECT_SOURCE_DIR}/src/Scripting.cpp
        ${PROJECT_SOURCE_DIR}/src/Scripting.hpp
        ${PROJECT_SOURCE_DIR}/src/UI.hpp
        ${PROJECT_SOURCE_DIR}/src/WorldSession.cpp
        ${PROJECT_SOURCE_DIR}/src/WorldSession.hpp
)

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Client"
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden)

add_dependencies(${PROJECT_NAME} StormGraphCommon)
target_link_libraries(${PROJECT_NAME} StormGraphCommon)

add_dependencies(${PROJECT_NAME} StormGraphCore)
target_link_libraries(${PROJECT_NAME} StormGraphCore)

add_dependencies(${PROJECT_NAME} Gui)
add_dependencies(${PROJECT_NAME} OpenGlDriver)

target_include_directories(${PROJECT_NAME} PUBLIC
    include
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

if (SDL_DLL)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL_DLL}" ${DIST_DIR}/bin)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" ${DIST_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${STORMGRAPH_ASSETS_DIR}" ${DIST_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GUI_ASSETS_DIR}" ${DIST_DIR})
