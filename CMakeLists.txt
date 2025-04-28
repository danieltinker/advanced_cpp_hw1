cmake_minimum_required(VERSION 3.10)
project(TankGame)

# Compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add warning flags exactly as you asked
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Source files
set(SOURCES
    main.cpp
    Board.cpp
    Tank.cpp
    GameState.cpp
    TankAlgorithm.cpp
)

# Header files (optional, for IDEs only)
set(HEADERS
    Board.h
    Tank.h
    GameState.h
    TankAlgorithm.h
)

# Executable target
add_executable(tank_game ${SOURCES} ${HEADERS})
