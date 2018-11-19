cmake_minimum_required(VERSION 3.1)
project(Gui)

set(CMAKE_CXX_STANDARD 14)
set(library ${PROJECT_NAME})

if (NOT TARGET StormGraphCore)
    add_subdirectory(../StormGraphCore ${CMAKE_BINARY_DIR}/build-StormGraphCore)
endif()

# Also add headers so that they're included in generated projects
set(sources
    ${PROJECT_SOURCE_DIR}/src/Radiance/EpicStyler.cpp
    ${PROJECT_SOURCE_DIR}/src/Radiance/Models.hpp
    ${PROJECT_SOURCE_DIR}/src/Radiance/OnyxStyler.cpp
    ${PROJECT_SOURCE_DIR}/src/Radiance/Radiance.cpp
    ${PROJECT_SOURCE_DIR}/src/Radiance/Styler.cpp
    ${PROJECT_SOURCE_DIR}/src/Radiance/Widget.cpp

    ${PROJECT_SOURCE_DIR}/src/UI/BoxSizer.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/Button.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/Driver.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/Gui.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/GuiDriver.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/GuiDriver.hpp
    ${PROJECT_SOURCE_DIR}/src/UI/Panel.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/PopupMenu.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/ProgressBar.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/ScrollBar.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/StaticText.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/TableLayout.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/TextBox.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/Widgets.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/Window.cpp
)

if (BUILD_SHARED_LIBS)
    add_library(${library} SHARED ${sources})
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Gui"
            C_VISIBILITY_PRESET hidden
            CXX_VISIBILITY_PRESET hidden)
else()
    add_library(${library} ${sources})
endif()

add_dependencies(${library} StormGraphCommon)
target_link_libraries(${library} StormGraphCommon)

add_dependencies(${library} StormGraphCore)
target_link_libraries(${library} StormGraphCore)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Export Assets
set(GUI_ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets PARENT_SCOPE)
