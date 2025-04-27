#include "Board.h"
#include "GameState.h"
#include "Tank.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "TankAlgorithm.h"


void clear_screen() {
    printf("\033[2J\033[H");
}

char get_key() {
    struct termios oldt, newt;
    char ch;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    
    newt.c_lflag &= ~(ICANON | ECHO); 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    read(STDIN_FILENO, &ch, 1);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: tanks_game <board_file>\n";
        return 1;
    }

    try {
        Board board(argv[1]);
        GameState game(board);
        board.print(game.tank1.getDirection(), game.tank2.getDirection());
        std::vector<std::string> moves;

        int i =1;
        moves.push_back("Start\n" + game.render());
        while (!game.isGameOver() && i<=200) {
            std::string s = "";
            auto tank1Position = game.getTank1Position();
            auto tank2Position = game.getTank2Position();
            auto tank1Direction = game.tank1.getDirection();
            auto tank2Direction = game.tank2.getDirection();
            auto tank1Cooldown = game.tank1.shootCooldown;

            std::string msg;
            Action p1 = decideTank1(board.grid, tank1Position, tank2Position, tank1Cooldown, tank1Direction);
            Action p2 = decideTank2(board.grid, tank2Position, tank1Position, tank2Direction,game.shells);
            game.step(p1, p2);
            
            tank1Position = game.getTank1Position();
            tank2Position = game.getTank2Position();
            tank1Direction = game.tank1.getDirection();
            tank2Direction = game.tank2.getDirection();
            tank1Cooldown = game.tank1.shootCooldown;

            s += " Just Taken actions: " + toString(p1) + " " + toString(p2) + "\n";
            s += "newTanks1pos:" + std::to_string(tank1Position.first) + " " + std::to_string(tank1Position.second) + "\n";
            s += "Tanks2pos:" + std::to_string(tank2Position.first) + " " + std::to_string(tank2Position.second) + "\n";
            s += "Tank1cooldown:" + std::to_string(tank1Cooldown) + "\n";
            s += "Tank1LOF:" + std::string(hasLineOfSight(board.grid, tank1Position, tank2Position) ? "true" : "false") + "\n";

            s +=game.render();
            moves.push_back(s);
            cout << "Turn "  << i << " complete\n";
            i++;
        }
    
        size_t index = 0;
    
        while (1) {
            clear_screen();
            cout << "\n Turn #" << index << "\n" <<  moves[index];
            cout << "\n[← or → to navigate, q to quit]\n";
    
            char ch = get_key();
    
            if (ch == 'q') break;
            else if (ch == '\033') { // Escape
                get_key(); 
                ch = get_key();
                if (ch == 'C') { // Right arrow
                    if (index < moves.size() - 1) index++;
                } else if (ch == 'D') { // Left arrow
                    if (index > 0) index--;
                }
            }
        }
    
        clear_screen();
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

}

    