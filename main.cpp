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
            auto tank2Cooldown = game.tank2.shootCooldown;
            Action p1 = decideTank1(board.grid, tank1Position, tank1Direction, tank1Cooldown, tank2Position);
            Action p2 = decideTank2(board.grid, tank2Position, tank2Direction, tank2Cooldown, tank1Position,game.shells);
            bool over = game.step(p1, p2);
            
            s += "Taken actions: " + toString(p1) + " " + toString(p2) + "\n";
            s +=game.render();
            moves.push_back(s);
            cout << "Turn "  << i << " complete\n";
            i++;
        }
    
        int index = 0;
    
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

    