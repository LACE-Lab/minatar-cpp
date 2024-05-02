include(CMakePrintHelpers)

cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(MinAtarCPP)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

find_package(PythonLibs 3 REQUIRED)

add_library(minatar-cpp MinAtarInterface.cpp)

target_include_directories(minatar-cpp PUBLIC ${PYTHON_INCLUDE_DIRS})
target_link_libraries(minatar-cpp PUBLIC ${PYTHON_LIBRARIES})


