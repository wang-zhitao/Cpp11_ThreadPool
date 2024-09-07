cmake_minimum_required(VERSION 3.12)
project(test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thread)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_subdirectory(thread)

add_executable(test main.cpp)
target_link_libraries(test THREADPOOL)