cmake_minimum_required(VERSION 3.21)
project(shairport-mqtt CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PahoMqttCpp CONFIG REQUIRED)

file(GLOB_RECURSE SHAIRPORT_MQTT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(shairport-mqtt ${SHAIRPORT_MQTT_SOURCES})
target_include_directories(shairport-mqtt PRIVATE include)
target_link_libraries(shairport-mqtt PRIVATE PahoMqttCpp::paho-mqttpp3)
