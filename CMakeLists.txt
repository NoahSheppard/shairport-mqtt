cmake_minimum_required(VERSION 3.21)
project(shairport-mqtt CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PahoMqttCpp CONFIG REQUIRED)

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp")

add_library(shairport-mqtt-core STATIC ${CORE_SOURCES})
target_include_directories(shairport-mqtt-core PUBLIC include)
target_link_libraries(shairport-mqtt-core PUBLIC PahoMqttCpp::paho-mqttpp3)

file(GLOB_RECURSE CLI_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/*.cpp")

add_executable(shairport-mqtt ${CLI_SOURCES})
target_link_libraries(shairport-mqtt PRIVATE shairport-mqtt-core)

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/*.cpp")

add_executable(shairport-mqtt-gui ${GUI_SOURCES})
target_link_libraries(shairport-mqtt-gui PRIVATE shairport-mqtt-core imgui::imgui glfw OpenGL::GL)

if(WIN32)
    target_link_options(shairport-mqtt-gui PRIVATE -mwindows)
endif()
