cmake_minimum_required(VERSION 3.12)
project(threadPool)

file(GLOB SRC  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
# add_library(THREADPOOL SHARED ${SRC})
add_library(THREADPOOL STATIC ${SRC})
target_link_libraries(THREADPOOL pthread)