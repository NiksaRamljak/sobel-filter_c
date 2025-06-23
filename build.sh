#!/bin/bash

#quick build script, use before the stats script
mkdir -p build
cd build
cmake ..
cmake --build .
