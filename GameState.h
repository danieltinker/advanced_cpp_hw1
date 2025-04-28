#pragma once

#include "Board.h"
#include "Tank.h"
#include <set>
#include <map>
#include <vector>
#include <utility>
#include <fstream> // for std::ofstream


using namespace std;
struct Shell {
    int x, y;
    Direction dir;
};

class GameState {
    friend int main(int argc, char* argv[]); 
    public:
    GameState(Board& board, const std::string& inputFilename); 

    bool step(Action p1Action, Action p2Action);
    std::string render() const;

    // Utility interface
    std::pair<int, int> getTank1Position() const;
    std::pair<int, int> getTank2Position() const;
    std::string getResult() const;
    bool isGameOver() const;

    // Modular step helpers
    void handleTankMineCollisions();
    void updateTankCooldowns();
    void applyTankActions(Action p1Action, Action p2Action);
    void confirmBackwardMoves();
    void updateTankPositionsOnBoard();
    void updateShellsWithOverrunCheck();
    void resolveShellCollisions();
    void filterRemainingShells();
    void handleTankShooting(Action p1Action, Action p2Action);
    void checkGameEndConditions(Action p1Action, Action p2Action);
    bool handleShellMidStepCollision(int x, int y);


private:
    Board& board;
    Tank tank1;
    Tank tank2;
    std::vector<Shell> shells;
    std::set<size_t> toRemove;
    std::map<std::pair<int, int>, std::vector<size_t>> positionMap;
    int stepCounter = 0; 
    // Internal logic
    void applyAction(Tank& tank, Action action);
    std::pair<int, int> findTank(CellContent tankSymbol);
    std::ofstream logFile;
};