cmake_minimum_required(VERSION 3.1)
project(OpenGlDriver)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(library ${PROJECT_NAME})

set(BUILD_SHARED_LIBS 0)

if (NOT TARGET StormGraphCore)
    add_subdirectory(../StormGraphCore ${CMAKE_BINARY_DIR}/build-StormGraphCore)
endif()

add_subdirectory(dependencies/freetype2)

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/OpenGlDriver/*.cpp
    ${PROJECT_SOURCE_DIR}/src/RenderingKit/gameui/*.cpp

    ${PROJECT_SOURCE_DIR}/src/OpenGlDriver/*.hpp
)

add_library(${library} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Driver.OpenGl"
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden)

add_dependencies(${library} StormGraphCommon)
target_link_libraries(${library} StormGraphCommon)

add_dependencies(${library} StormGraphCore)
target_link_libraries(${library} StormGraphCore)

# freetype
add_dependencies(${library} freetype)
target_link_libraries(${library} freetype)
target_include_directories(${library} PRIVATE "${PROJECT_SOURCE_DIR}/dependencies/freetype2/include")

# OpenGL
if (WIN32)
    target_link_libraries(${library} opengl32)
else()
    target_link_libraries(${library} GL)
endif()

# SDL
if (MSVC)
    set(SDL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies/SDL-1.2.15/include)
    set(SDL_LIBRARY ${PROJECT_SOURCE_DIR}/dependencies/SDL-1.2.15/lib/x86/SDL.lib)
    set(SDL_DLL ${PROJECT_SOURCE_DIR}/dependencies/SDL-1.2.15/lib/x86/SDL.dll PARENT_SCOPE)
endif()

find_package(SDL REQUIRED)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE "${SDL_INCLUDE_DIR}")
target_link_libraries(${PROJECT_NAME} "${SDL_LIBRARY}")

# Export Assets
set(OPENGLDRIVER_ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets PARENT_SCOPE)
