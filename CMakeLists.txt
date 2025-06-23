cmake_minimum_required(VERSION 3.10)
project(sobel_filter_c C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include headers directory globally for targets that need it
include_directories(${PROJECT_SOURCE_DIR}/headers)

# Minimal executable: fltr_sobel (standalone, only fltr_sobel.c, no external deps)
add_executable(fltr_sobel
    src/fltr_sobel.c
)

# Link math library for fltr_sobel (required for sqrt, etc)
if(NOT MSVC)
    target_link_libraries(fltr_sobel PRIVATE m)
endif()

# Full-feature executable: fltr_sobel_test with modular sources and threading/JPEG
add_executable(fltr_sobel_test
    src/fltr_sobel_test.c
    src/jpeg.c
    src/pgm.c
    src/thread_auto.c
)

# Compile definitions for fltr_sobel_test to enable features
target_compile_definitions(fltr_sobel_test PRIVATE WITH_JPEG=1 W_AUTOTHREAD=1)

# Link math lib to fltr_sobel_test as well (for sqrt)
if(NOT MSVC)
    target_link_libraries(fltr_sobel_test PRIVATE m)
endif()

# Find TurboJPEG library for fltr_sobel_test (optional, warning if missing)
find_library(TURBOJPEG_LIB turbojpeg)

if(TURBOJPEG_LIB)
    target_link_libraries(fltr_sobel_test PRIVATE ${TURBOJPEG_LIB})
else()
    message(WARNING "TurboJPEG library not found. JPEG support disabled for fltr_sobel_test.")
endif()
