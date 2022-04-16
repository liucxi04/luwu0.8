cmake_minimum_required(VERSION 2.8.3)
project(luwu)

find_package(Threads)
find_package(Boost REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(luwu ${Boost_INCLUDE_DIRS} ${YAML_CPP_INCLUDE_DIR})

link_directories(${Boost_LIBRARY_DIRS} ${YAML_CPP_LIBRARY_DIR})

set(SRC
        luwu/log.cpp
        luwu/utils.cpp
        luwu/config.cpp
        luwu/thread.cpp)
add_library(luwu SHARED ${SRC})

add_executable(log tests/test_log.cpp)
add_dependencies(log luwu)
target_link_libraries(log luwu ${CMAKE_THREAD_LIBS_INIT})

add_executable(config tests/test_config.cpp)
add_dependencies(config luwu)
target_link_libraries(config luwu ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARICES} yaml-cpp)

add_executable(thread tests/test_thread.cpp)
add_dependencies(thread luwu)
target_link_libraries(thread luwu ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARICES} yaml-cpp)