#include "Board.h"
#include "GameState.h"
#include "Tank.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: tanks_game <board_file>\n";
        return 1;
    }

    try {
        Board board(argv[1]);
        GameState game(board);

        // while (!game.isGameOver()) {
        for (int i = 0; i < 20; ++i) {
            auto [x1, y1] = game.getTank1Position();
            auto [x2, y2] = game.getTank2Position();
            Action p1 = game.chaseTarget(x1, y1, x2, y2);
            Action p2 = Action::MOVE_FORWARD;

            game.step(p1, p2);
            game.render();
            std::cout << "---\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
