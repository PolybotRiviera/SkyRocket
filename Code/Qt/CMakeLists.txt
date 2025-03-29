cmake_minimum_required(VERSION 3.16)

project(SkyRocket VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(Qt6 COMPONENTS Widgets Bluetooth REQUIRED)

message("Qt6Widgets_DIR: ${Qt6Widgets_DIR}")
message("Qt6Bluetooth_DIR: ${Qt6Bluetooth_DIR}")

qt6_standard_project_setup()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

include_directories(include)

qt6_add_resources(resource.qrc)

qt6_add_executable(
    SkyRocket
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
)

target_link_libraries(SkyRocket PRIVATE Qt6::Widgets Qt6::Bluetooth)

# Windows-specific settings for Bluetooth
if(WIN32)
    set_target_properties(SkyRocket PROPERTIES
        WIN32_EXECUTABLE TRUE
        QT_WIN32_CAPABILITIES "bluetooth"
    )
endif()
