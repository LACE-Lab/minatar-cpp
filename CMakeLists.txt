include(CMakePrintHelpers)

cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(MinAtarCPP)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

add_library(minatar-cpp MinAtarInterface.cpp)

target_include_directories(minatar-cpp PUBLIC ${Python3_INCLUDE_DIRS})
target_link_libraries(minatar-cpp PUBLIC ${Python3_LIBRARIES})


