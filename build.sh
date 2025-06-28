#!/bin/bash

#quick build script, use before the stats scripts
mkdir -p build
cd build
cmake ..
cmake --build .
