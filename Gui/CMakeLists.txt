cmake_minimum_required(VERSION 3.1)
project(Gui)

set(CMAKE_CXX_STANDARD 14)
set(library ${PROJECT_NAME})

if (NOT TARGET StormGraphCore)
    add_subdirectory(../StormGraphCore ${CMAKE_BINARY_DIR}/build-StormGraphCore)
endif()

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/UI/*.cpp

    ${PROJECT_SOURCE_DIR}/src/UI/*.hpp
)

add_library(${library} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Gui"
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden)

add_dependencies(${library} StormGraphCommon)
target_link_libraries(${library} StormGraphCommon)

add_dependencies(${library} StormGraphCore)
target_link_libraries(${library} StormGraphCore)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Export Assets
set(GUI_ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets PARENT_SCOPE)
