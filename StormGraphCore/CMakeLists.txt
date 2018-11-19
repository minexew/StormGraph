cmake_minimum_required(VERSION 3.1)
project(StormGraphCore)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(library ${PROJECT_NAME})

if (NOT TARGET StormGraphCommon)
    add_subdirectory(../StormGraphCommon ${CMAKE_BINARY_DIR}/build-StormGraphCommon)
endif()

set(BUILD_SHARED_LIBS 0)

# controls libjpeg
set(BUILD_STATIC CACHE BOOL ON)

add_subdirectory(dependencies/libjpeg ${CMAKE_CURRENT_BINARY_DIR}/build-libjpeg)
add_subdirectory(dependencies/ogg)
add_subdirectory(dependencies/openal-soft)

set(OGG_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/dependencies/ogg/include)
set(OGG_LIBRARIES $<TARGET_FILE:ogg>)
add_subdirectory(dependencies/vorbis)

add_subdirectory(dependencies/zlib-ng ${CMAKE_CURRENT_BINARY_DIR}/build-zlib-ng)

set(ZLIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/dependencies/zlib-ng ${CMAKE_CURRENT_BINARY_DIR}/build-zlib)
add_subdirectory(dependencies/Moxillan2)

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/Common/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Core/*.cpp
    ${PROJECT_SOURCE_DIR}/src/IO/*.cpp

    ${PROJECT_SOURCE_DIR}/dependencies/lodepng/lodepng.cpp

    ${PROJECT_SOURCE_DIR}/include/StormGraph/*.hpp
    ${PROJECT_SOURCE_DIR}/include/StormGraph/IO/*.hpp
)

if (BUILD_SHARED_LIBS)
    add_library(${library} SHARED ${sources})
    set_target_properties(${library} PROPERTIES OUTPUT_NAME "Core"
            C_VISIBILITY_PRESET hidden
            CXX_VISIBILITY_PRESET hidden)
else()
    add_library(${library} ${sources})

    target_compile_definitions(${library} PUBLIC -DStormGraph_Static_Core=1)

    # This *obviously* doesn't belong here, but who cares at this point?
    target_compile_definitions(${library} PUBLIC -DStormGraph_Static_GraphicsDriver="OpenGl")
    target_compile_definitions(${library} PUBLIC -DStormGraph_Static_GuiDriver="OpenGl")
endif()

add_dependencies(${library} StormGraphCommon)
target_link_libraries(${library} StormGraphCommon)

# jpeg
add_dependencies(${library} jpeg)
target_link_libraries(${library} jpeg)
target_include_directories(${library} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/libjpeg)
target_include_directories(${library} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/build-libjpeg)

# Moxillan
add_dependencies(${library} Moxillan)
target_link_libraries(${library} Moxillan)

# ogg
add_dependencies(${library} ogg)
target_link_libraries(${library} ogg)

# openal-soft
if (WIN32)
    add_dependencies(${library} OpenAL32)
    target_link_libraries(${library} OpenAL32)
else()
    add_dependencies(${library} openal)
    target_link_libraries(${library} openal)
endif()

# vorbis
add_dependencies(${library} vorbis)
target_link_libraries(${library} vorbis)

# vorbisfile
add_dependencies(${library} vorbisfile)
target_link_libraries(${library} vorbisfile)

if (UNIX)
    target_link_libraries(${library} dl)
    target_link_libraries(${library} pthread)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    include
)

target_include_directories(${PROJECT_NAME} PRIVATE
    dependencies/lodepng
    dependencies/ogg/include
    dependencies/openal-soft/include/AL
    dependencies/vorbis/include
)

# Export Assets
set(STORMGRAPH_ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets PARENT_SCOPE)
