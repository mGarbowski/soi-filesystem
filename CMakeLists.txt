cmake_minimum_required(VERSION 3.27)
project(soi_filesystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(soi_filesystem src/main.cpp
        src/common.cpp
        src/common.h
        src/FreeBlocksBitmap.cpp
        src/FreeBlocksBitmap.h
        src/Superblock.cpp
        src/Superblock.h
        src/INode.cpp
        src/INode.h
        src/Directory.cpp
        src/Directory.h
        src/VirtualDisk.cpp
        src/VirtualDisk.h)
