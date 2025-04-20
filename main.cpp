#include "Board.h"
#include "GameState.h"
#include "Tank.h"
#include <iostream>
#include "tankAlgorithm.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: tanks_game <board_file>\n";
        return 1;
    }

    try {
        Board board(argv[1]);
        GameState game(board);
        board.print(game.tank1.getDirection(), game.tank2.getDirection()); // TODO, board should have it
            
        while (!game.isGameOver()) {
            auto tank1Position = game.getTank1Position();
            auto tank2Position = game.getTank2Position();
            auto tank1Direction = game.tank1.getDirection();
            auto tank1Cooldown = game.tank1.shootCooldown;
            Action p1 = decideTank1(board.grid, tank1Position, tank1Direction, tank1Cooldown, tank2Position);//Action::NONE;//
            auto tank2Direction = game.tank2.getDirection();
            auto tank2Cooldown = game.tank2.shootCooldown;
            Action p2 = decideTank2(board.grid, tank2Position, tank2Direction, tank2Cooldown, tank1Position);//Action::NONE;//
            bool over = game.step(p1, p2);
            std::cout << "Taken actions: " << toString(p1) << " " << toString(p2) << "\n";
            game.render();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
