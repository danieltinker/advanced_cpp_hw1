# Tank Battle Simulator
A C++ project that simulates a two-player tank battle on a dynamic board.
Each player controls a tank that can move, rotate, shoot, and interact with mines, walls, and shells.

## Project Structure

File	        Purpose
main.cpp	       Runs the game loop
Board.h            Board.cpp	Manages the 2D board state
Tank.h             Tank.cpp	Represents tank movement, shooting, and cooldowns
GameState.h        GameState.cpp	Controls the game rules, turns, and collisions
TankAlgorithm.h    TankAlgorithm.cpp	Algorithms for tank decision making (chase/reactive)
CMakeLists.txt     Build configuration


## How to Build
This project uses CMake for easy compilation.

Quick build instructions:
git clone https://github.com/danieltinker/advanced_cpp_hw1.git
cd advanced_cpp_hw1
mkdir build
cd build
cmake ..
make

After building, you'll have an executable called:
./tank_game


## How to Run (inputs)
./tank_game <board_file_path>.txt

Example:
./tank_game ../input_a.txt

Where <board_file_path>.txt is a text file representing the initial state of the game board.

## Board File Format
Example:

7 5
#######
#1  @ #
# ### #
#   2 #
#######

Symbol	Meaning
\#	Wall (takes 2 hits to destroy)
@	Mine (destroys tank immediately if stepped on)
1	Player 1's tank (starts facing Left)
2	Player 2's tank (starts facing Right)
Empty space



## Game Logging (Outputs):

Output written to output_<inputfile>.txt:
- All requested actions (even invalid ones)
- Step-by-step game state
- Final game result (win/tie)

Input Errors Log:
-  If input file has recoverable errors (e.g. extra tanks, wrong dimensions, unknown symbols),
details are written into input_errors.txt (only if errors exist).

## Game Rules:
- Tanks can move forward, rotate, shoot, or move backward (with delay).
- Shells move twice as fast as tanks.
- Stepping on a mine destroys a tank instantly.
- If tanks collide with each other, both are destroyed.
- If both tanks run out of ammo and 40 more turns pass, the game ends in a tie.


## Contributors
Daniel Baruch 315634022
Evyatar Oren 331684530



