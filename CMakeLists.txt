cmake_minimum_required(VERSION 3.17)
project(rtx_weekend)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(rtx_weekend main.cpp Ray.h Vector.h Sphere.h hittest.h math.h Material.cpp Material.h HitRecord.h Vector.cpp Plane.cpp Plane.h Box.cpp Box.h ThreadPool.cpp ThreadPool.h Camera.cpp Camera.h math.cpp)