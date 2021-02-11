cmake_minimum_required(VERSION 3.10)

project(convex-hull)

link_libraries("-lmingw32 -lSDL2main -lSDL2.dll -lSDL2_gfx.dll")
add_executable(convex-hull main.cpp)
set(CMAKE_C_FLAGS, "-g -O0")
