cmake_minimum_required(VERSION 3.20)

project(hotrod)

add_executable(${PROJECT_NAME} main.c dock.c systray.c)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
include(FindPkgConfig)
pkg_search_module(XCB REQUIRED xcb)
include_directories(${XCB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${XCB_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)