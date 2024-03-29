cmake_minimum_required(VERSION 3.1)
project(StormCraft)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 14)
set(DIST_DIR ${CMAKE_SOURCE_DIR}/dist)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_subdirectory(../StormGraphCore  ${CMAKE_BINARY_DIR}/build-StormGraphCore)
add_subdirectory(../Gui             ${CMAKE_BINARY_DIR}/build-Gui)
add_subdirectory(../OpenGlDriver    ${CMAKE_BINARY_DIR}/build-OpenGLDriver)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DIST_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DIST_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/ContentTools/*.cpp
    ${PROJECT_SOURCE_DIR}/src/StormCraft/*.cpp

    ${PROJECT_SOURCE_DIR}/src/StormCraft/*.hpp
)

find_package(wxWidgets REQUIRED COMPONENTS core base aui html gl propgrid)
include(${wxWidgets_USE_FILE})

add_executable(${PROJECT_NAME} ${sources})
# Workaround to avoid filename clash on linux
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "StormCraft_editor")

add_dependencies(${PROJECT_NAME} StormGraphCommon)
target_link_libraries(${PROJECT_NAME} StormGraphCommon)

add_dependencies(${PROJECT_NAME} StormGraphCore)
target_link_libraries(${PROJECT_NAME} StormGraphCore)

add_dependencies(${PROJECT_NAME} Gui)
target_link_libraries(${PROJECT_NAME} Gui)

add_dependencies(${PROJECT_NAME} OpenGlDriver)
target_link_libraries(${PROJECT_NAME} OpenGlDriver)

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${STORMGRAPH_ASSETS_DIR}" ${DIST_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${GUI_ASSETS_DIR}" ${DIST_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" ${DIST_DIR})
