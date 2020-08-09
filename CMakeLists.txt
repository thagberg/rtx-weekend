cmake_minimum_required(VERSION 3.17)
project(rtx_weekend)

set(CMAKE_CXX_STANDARD 20)

add_executable(rtx_weekend main.cpp Ray.h Vector.h Sphere.h hittest.h)