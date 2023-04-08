cmake_minimum_required(VERSION 3.20)
project(LogCPP)

include_directories(include)

add_executable(test src/test.cpp)
add_library(LogCPP SHARED src/LogStream.cpp src/Elements.cpp src/Loggers.cpp)
add_library(JsonCPP SHARED src/JSON.cpp)
target_link_libraries(LogCPP JsonCPP)
target_link_libraries(test LogCPP)
