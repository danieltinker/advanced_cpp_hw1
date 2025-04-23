cmake_minimum_required(VERSION 3.10)
project(TankGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    main.cpp
    Board.cpp
    Tank.cpp
    GameState.cpp
    TankAlgorithm.cpp
)

# Header files (for IDE clarity, not required for build)
set(HEADERS
    Board.h
    Tank.h
    GameState.h
    TankAlgorithm.h
)

# Executable target
add_executable(tank_game ${SOURCES} ${HEADERS})