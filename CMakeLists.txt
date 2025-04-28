cmake_minimum_required(VERSION 3.10)
project(TankGame)

# Compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add warning flags exactly as required
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Check GCC version >= 11.4
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.4)
        message(FATAL_ERROR "GCC version must be at least 11.4!")
    endif()
endif()

# Source files
set(SOURCES
    main.cpp
    Board.cpp
    Tank.cpp
    GameState.cpp
    TankAlgorithm.cpp
)

# Header files (optional, just for IDE clarity)
set(HEADERS
    Board.h
    Tank.h
    GameState.h
    TankAlgorithm.h
)

# Executable target
add_executable(tank_game ${SOURCES} ${HEADERS})
