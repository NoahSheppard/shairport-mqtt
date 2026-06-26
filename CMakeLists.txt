cmake_minimum_required(VERSION 3.21)
project(shairport-mqtt CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PahoMqttCpp CONFIG REQUIRED)

add_executable(shairport-mqtt src/main.cpp)
target_link_libraries(shairport-mqtt PRIVATE PahoMqttCpp::paho-mqttpp3)
