Depends on:
* libjpeg-turbo
* cmake
* perf

tested only on linux

**fltr_sobel** is the bare c11 program without external libraries
**fltr_sobel_test** has jpeg and dynamic thread setting support, is also more modular in its design

**build.sh** builds the project

**performance_full.sh** tests the performance of fltr_sobel_test

**performance_noext.sh** tests the performance of fltr_sobel
both output to ./performance_logs
