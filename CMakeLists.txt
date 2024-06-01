cmake_minimum_required(VERSION 3.10)

project(ecs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})

add_library(ecs STATIC src/ECS.cpp)

