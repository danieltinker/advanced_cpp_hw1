// ===============================
// File: GameState.h
// Interface for GameState class
// ===============================

#pragma once

#include "Board.h"
#include "Tank.h"
#include <utility>
#include <vector>

using namespace std;

struct Shell {
    int x, y;
    Direction dir;
};

class GameState {
    friend int main(int argc, char* argv[]); // TODO remove
public:
    GameState(Board& board);

    // Primary step update function
    bool step(Action p1Action, Action p2Action);
    std::string render() const;

    // Utility interface
    std::pair<int, int> getTank1Position() const;
    std::pair<int, int> getTank2Position() const;
    std::string getResult() const;
    bool isGameOver() const;

private:
    Board& board;
    Tank tank1;
    Tank tank2;
    std::vector<Shell> shells;

    // Internal logic
    void applyAction(Tank& tank, Action action);
    std::pair<int, int> findTank(CellContent tankSymbol);
};