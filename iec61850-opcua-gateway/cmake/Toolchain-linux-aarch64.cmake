# CMake Toolchain file for cross-compiling to Linux aarch64 (arm64)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross compiler
set(TOOLCHAIN_PREFIX aarch64-linux-gnu)

# Cross compilers
find_program(CMAKE_C_COMPILER NAMES ${TOOLCHAIN_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}-g++)
find_program(CMAKE_AR NAMES ${TOOLCHAIN_PREFIX}-ar)
find_program(CMAKE_RANLIB NAMES ${TOOLCHAIN_PREFIX}-ranlib)
find_program(CMAKE_STRIP NAMES ${TOOLCHAIN_PREFIX}-strip)

if(NOT CMAKE_C_COMPILER OR NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "aarch64 cross-compiler not found. Install with: sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu")
endif()

# Target environment
set(CMAKE_FIND_ROOT_PATH 
    /usr/aarch64-linux-gnu
    /usr/lib/aarch64-linux-gnu
)

# Adjust the behavior of FIND_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Compiler options for compatibility
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a")

# Set executable suffix (none for Linux)
set(CMAKE_EXECUTABLE_SUFFIX "")
