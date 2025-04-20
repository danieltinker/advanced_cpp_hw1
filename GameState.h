// ===============================
// File: GameState.h
// Interface for GameState class
// ===============================

#pragma once

#include "Board.h"
#include "Tank.h"
#include <utility>
#include <vector>

struct Shell {
    int x, y;
    Direction dir;
};

class GameState {
public:
    GameState(Board& board);

    // Primary step update function
    void step(Action p1Action, Action p2Action);
    void render() const;

    // Utility interface
    Action chaseTarget(int srcX, int srcY, int tgtX, int tgtY);
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