cmake_minimum_required(VERSION 3.1)
project(TolClient)

set(CMAKE_CXX_STANDARD 14)

set(DIST_DIR ${CMAKE_SOURCE_DIR}/dist)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_subdirectory(../StormGraphCore  ${CMAKE_BINARY_DIR}/build-StormGraphCore)
add_subdirectory(../Gui             ${CMAKE_BINARY_DIR}/build-Gui)
add_subdirectory(../OpenGlDriver    ${CMAKE_BINARY_DIR}/build-OpenGLDriver)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DIST_DIR}/${PROJECT_NAME}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR}/${PROJECT_NAME}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB sources
        ${PROJECT_SOURCE_DIR}/src/InitScene.cpp
        ${PROJECT_SOURCE_DIR}/src/InitScene.hpp
        ${PROJECT_SOURCE_DIR}/src/TitleScene.cpp
        ${PROJECT_SOURCE_DIR}/src/TitleScene.hpp
        ${PROJECT_SOURCE_DIR}/src/TolClient.cpp
        ${PROJECT_SOURCE_DIR}/src/TolClient.hpp
        ${PROJECT_SOURCE_DIR}/src/WorldScene.cpp
        ${PROJECT_SOURCE_DIR}/src/WorldScene.hpp
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
target_link_libraries(${PROJECT_NAME} Gui)          # probably not the best way to get just the include paths

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
