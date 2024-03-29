cmake_minimum_required(VERSION 3.1)
project(Orion)

set(CMAKE_CXX_STANDARD 14)

set(DIST_DIR ${CMAKE_SOURCE_DIR}/dist)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_subdirectory(../StormGraphCore  ${CMAKE_BINARY_DIR}/build-StormGraphCore)
add_subdirectory(../Gui             ${CMAKE_BINARY_DIR}/build-Gui)
add_subdirectory(../OpenGlDriver    ${CMAKE_BINARY_DIR}/build-OpenGLDriver)

if (BUILD_SHARED_LIBS)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DIST_DIR}/${PROJECT_NAME}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR}/${PROJECT_NAME}/bin)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR})
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.hpp
)

set(sources
        ${PROJECT_SOURCE_DIR}/src/GameScene.cpp
        ${PROJECT_SOURCE_DIR}/src/GameScene.hpp
        ${PROJECT_SOURCE_DIR}/src/MenuScene.cpp
        ${PROJECT_SOURCE_DIR}/src/MenuScene.hpp
        ${PROJECT_SOURCE_DIR}/src/Orion.cpp
)

if (BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${sources})
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Client"
            C_VISIBILITY_PRESET hidden
            CXX_VISIBILITY_PRESET hidden)
else()
    add_executable(${PROJECT_NAME} ${sources})

    # Workaround to avoid filename clash on linux
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "oriongame")
endif()

add_dependencies(${PROJECT_NAME} StormGraphCommon)
target_link_libraries(${PROJECT_NAME} StormGraphCommon)

add_dependencies(${PROJECT_NAME} StormGraphCore)
target_link_libraries(${PROJECT_NAME} StormGraphCore)

add_dependencies(${PROJECT_NAME} Gui)
add_dependencies(${PROJECT_NAME} OpenGlDriver)

if (NOT BUILD_SHARED_LIBS)
    target_link_libraries(${PROJECT_NAME} Gui)
    target_link_libraries(${PROJECT_NAME} OpenGlDriver)
endif()

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
