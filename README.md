# Simple filesystem
Mikołaj Garbowski

Assignment for Operating Systems course @ Warsaw University of Technology

## Overview
Project partially implements a simple filesystem based on UNIX.
Limited to exchanging files between native filesystem and the root directory of the virtual filesystem.

## Build
```shell
mkdir build
cmake -S . -B build/
cmake --build build/
```

## Run
```shell
./build/soi_filesystem [virtual disk path]
```