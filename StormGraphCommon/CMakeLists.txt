cmake_minimum_required(VERSION 3.1)
project(StormGraphCommon)

set(CMAKE_CXX_STANDARD 14)
set(library ${PROJECT_NAME})

set(BUILD_SHARED_LIBS 0)

add_subdirectory(dependencies/libcfx2)
add_subdirectory(dependencies/littl)

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/Common/*.cpp

    ${PROJECT_SOURCE_DIR}/include/StormGraph/*.hpp
)

add_library(${library} SHARED ${sources})
set_target_properties(${library} PROPERTIES OUTPUT_NAME "Common"
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden)

target_compile_definitions(${library} PRIVATE -DStormGraph_Build_Common_DLL=1)

# libcfx2-0.8
add_dependencies(${library} cfx2_08)
target_link_libraries(${library} PUBLIC cfx2_08)

# littl
add_dependencies(${library} littl)
target_link_libraries(${library} PUBLIC littl)

target_include_directories(${PROJECT_NAME} PUBLIC
    include
    dependencies/glm
)
