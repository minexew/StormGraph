cmake_minimum_required(VERSION 3.1)
project(Moxillan)

set(CMAKE_CXX_STANDARD 14)
set(library ${PROJECT_NAME})

# Also add headers so that they're included in generated projects
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/*.cpp

    ${PROJECT_SOURCE_DIR}/include/Moxillan/*.hpp
)

add_library(${library} STATIC ${sources})

target_include_directories(${library} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${library} PRIVATE ${ZLIB_INCLUDE_DIR})

# zlib
add_dependencies(${library} zlibstatic)
target_link_libraries(${library} zlibstatic)

add_dependencies(${library} littl)
target_link_libraries(${library} littl)
