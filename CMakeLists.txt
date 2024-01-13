cmake_minimum_required(VERSION 3.27)
project(soi_filesystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(soi_filesystem src/main.cpp
        src/common.cpp
        src/FreeBlocksBitmap.cpp
        src/FreeBlocksBitmap.h
        src/INode.cpp
        src/INode.h
        src/Directory.cpp
        src/Directory.h)
