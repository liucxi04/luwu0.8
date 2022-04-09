cmake_minimum_required(VERSION 2.8.3)
project(luwu)

find_package(Threads)
find_package(Boost REQUIRED)
#find_package(yaml-cpp REQUIRED)

include_directories(luwu)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${YAML_CPP_INCLUDE_DIR})

link_directories(${Boost_LIBRARY_DIRS})
#include_directories(${YAML_CPP_LIBRARY_DIR})

set(SRC
        luwu/log.cpp
        luwu/utils.cpp
        luwu/config.cpp)
add_library(luwu SHARED ${SRC})

add_executable(log_config tests/test_config.cpp)
add_dependencies(log_config luwu)
target_link_libraries(log_config luwu)


target_link_libraries(log_config ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(log_config ${Boost_LIBRARICES})
target_link_libraries(log_config yaml-cpp)