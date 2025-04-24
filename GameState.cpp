#include "GameState.h"
#include <map>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include <cmath>


using namespace std;
// --- Global Game State Flags ---
int emptyAmmoSteps = 0;
bool gameOver = false;
std::string gameResult;
std::ofstream logFile("game_output.txt");


GameState::GameState(Board& board): board(board),
      tank1([&] { auto [x1, y1] = findTank(CellContent::TANK1); return Tank(1, x1, y1, Direction::L); }()),
      tank2([&] { auto [x2, y2] = findTank(CellContent::TANK2); return Tank(2, x2, y2, Direction::R); }()) {}





void GameState::applyAction(Tank& tank, Action action) {
    if (tank.isWaitingToMoveBack()) return;

    switch (action) {
        case Action::MOVE_FORWARD:
            tank.cancelBackwardRequest();
            tank.moveForward();
            break;
        case Action::MOVE_BACKWARD:
            tank.requestBackward();
            break;
        case Action::ROTATE_LEFT_EIGHTH:
            tank.rotateLeftEighth();
            break;
        case Action::ROTATE_RIGHT_EIGHTH:
            tank.rotateRightEighth();
            break;
        case Action::ROTATE_LEFT_QUARTER:
            tank.rotateLeftQuarter();
            break;
        case Action::ROTATE_RIGHT_QUARTER:
            tank.rotateRightQuarter();
            break;
        case Action::SHOOT:
            // shoot is handled after shell collisions
            break;
        default:
            break;
    }
}

bool GameState::step(Action p1Action, Action p2Action) {
    if (gameOver) return true;

    handleTankMineCollisions();
    updateTankCooldowns();
    applyTankActions(p1Action, p2Action);
    confirmBackwardMoves();
    updateTankPositionsOnBoard();
    updateShellsWithOverrunCheck();
    resolveShellCollisions();
    filterRemainingShells();
    handleTankShooting(p1Action, p2Action);
    checkGameEndConditions(p1Action, p2Action);

    return gameOver;
}

void GameState::handleTankMineCollisions() {
    auto [x1, y1] = tank1.getPosition();
    auto [x2, y2] = tank2.getPosition();
    if (tank1.isAlive() && board.getCell(x1, y1).content == CellContent::MINE) {
        tank1.destroy();
        board.setCell(x1, y1, CellContent::EMPTY);
    }
    if (tank2.isAlive() && board.getCell(x2, y2).content == CellContent::MINE) {
        tank2.destroy();
        board.setCell(x2, y2, CellContent::EMPTY);
    }
}

void GameState::updateTankCooldowns() {
    tank1.updateCooldowns();
    tank2.updateCooldowns();
}

void GameState::applyTankActions(Action p1Action, Action p2Action) {
    applyAction(tank1, p1Action);
    applyAction(tank2, p2Action);
}

void GameState::confirmBackwardMoves() {
    tank1.confirmBackwardMove();
    tank2.confirmBackwardMove();
}

void GameState::updateTankPositionsOnBoard() {
    board.clearTankMarks();
    auto [x1, y1] = tank1.getPosition();
    auto [x2, y2] = tank2.getPosition();

    // 💥 NEW: Tank-on-tank collision
    if (tank1.isAlive() && tank2.isAlive() &&
    tank1.getPosition() == tank2.getPosition()) {
    tank1.destroy();
    tank2.destroy();
    board.setCell(x1, y1, CellContent::EMPTY);
    }


    if (tank1.isAlive()) board.setCell(x1, y1, CellContent::TANK1);
    if (tank2.isAlive()) board.setCell(x2, y2, CellContent::TANK2);
}

void GameState::updateShellsWithOverrunCheck() {
    board.clearShellMarks();
    toRemove.clear();
    positionMap.clear();

    for (size_t i = 0; i < shells.size(); ++i) {
        int dx = 0, dy = 0;
        switch (shells[i].dir) {
            case Direction::U:  dy = -2; break;
            case Direction::UR: dx = 2; dy = -2; break;
            case Direction::R:  dx = 2; break;
            case Direction::DR: dx = 2; dy = 2; break;
            case Direction::D:  dy = 2; break;
            case Direction::DL: dx = -2; dy = 2; break;
            case Direction::L:  dx = -2; break;
            case Direction::UL: dx = -2; dy = -2; break;
        }

        shells[i].x += dx;
        shells[i].y += dy;
        board.wrapCoords(shells[i].x, shells[i].y);

        int midX = shells[i].x - dx / 2;
        int midY = shells[i].y - dy / 2;
        board.wrapCoords(midX, midY);

        if (tank1.isAlive() && midX == tank1.getPosition().first && midY == tank1.getPosition().second) {
            tank1.destroy();
            board.setCell(midX, midY, CellContent::EMPTY);
            toRemove.insert(i);
            continue;
        }
        if (tank2.isAlive() && midX == tank2.getPosition().first && midY == tank2.getPosition().second) {
            tank2.destroy();
            board.setCell(midX, midY, CellContent::EMPTY);
            toRemove.insert(i);
            continue;
        }

        positionMap[{shells[i].x, shells[i].y}].push_back(i);
    }
}

void GameState::resolveShellCollisions() {
    for (const auto& [pos, indices] : positionMap) {
        int x = pos.first, y = pos.second;
        auto cell = board.getCell(x, y);

        if (indices.size() > 1) {
            for (size_t i : indices) toRemove.insert(i);
            continue;
        }

        size_t i = indices[0];
        if (cell.content == CellContent::WALL) {
            board.grid[y][x].wallHits++;
            if (board.grid[y][x].wallHits >= 2)
                board.setCell(x, y, CellContent::EMPTY);
            toRemove.insert(i);
        } else if (cell.content == CellContent::TANK1 && tank1.isAlive()) {
            tank1.destroy();
            board.setCell(x, y, CellContent::EMPTY);
            toRemove.insert(i);
        } else if (cell.content == CellContent::TANK2 && tank2.isAlive()) {
            tank2.destroy();
            board.setCell(x, y, CellContent::EMPTY);
            toRemove.insert(i);
        }
    }
}

void GameState::filterRemainingShells() {
    std::vector<Shell> remaining;
    for (size_t i = 0; i < shells.size(); ++i) {
        if (toRemove.find(i) == toRemove.end()) {
            remaining.push_back(shells[i]);
            board.grid[shells[i].y][shells[i].x].hasShellOverlay = true;
        }
    }
    shells = std::move(remaining);
}

void GameState::handleTankShooting(Action p1Action, Action p2Action) {
    auto spawnShell = [&](Tank& tank) {
        auto [sx, sy] = tank.getPosition();
        int dx = 0, dy = 0;
        switch (tank.getDirection()) {
            case Direction::U:  dy = -1; break;
            case Direction::UR: dx = 1; dy = -1; break;
            case Direction::R:  dx = 1; break;
            case Direction::DR: dx = 1; dy = 1; break;
            case Direction::D:  dy = 1; break;
            case Direction::DL: dx = -1; dy = 1; break;
            case Direction::L:  dx = -1; break;
            case Direction::UL: dx = -1; dy = -1; break;
        }
        board.wrapCoords(sx += dx, sy += dy);
        shells.push_back({sx, sy, tank.getDirection()});
    };

    if (p1Action == Action::SHOOT && tank1.canShoot()) {
        tank1.shoot();
        spawnShell(tank1);
    }
    if (p2Action == Action::SHOOT && tank2.canShoot()) {
        tank2.shoot();
        spawnShell(tank2);
    }
}

void GameState::checkGameEndConditions(Action p1Action, Action p2Action) {
    if (!tank1.isAlive() && tank2.isAlive()) {
        gameOver = true;
        gameResult = "Player 2 wins (Player 1 destroyed)";
    } else if (!tank2.isAlive() && tank1.isAlive()) {
        gameOver = true;
        gameResult = "Player 1 wins (Player 2 destroyed)";
    } else if (!tank1.isAlive() && !tank2.isAlive()) {
        gameOver = true;
        gameResult = "Tie (Both tanks destroyed)";
    } else if (tank1.getShellCount() == 0 && tank2.getShellCount() == 0) {
        emptyAmmoSteps++;
        if (emptyAmmoSteps >= 40) {
            gameOver = true;
            gameResult = "Tie (40 steps after ammo exhausted)";
        }
    } else {
        emptyAmmoSteps = 0;
    }

    if (gameOver) {
        logFile << "Result: " << gameResult << "\n";
        logFile.close();
    }
    logFile << "STEP " << stepCounter++ << ":\n";
    logFile << "P1 requested: " << static_cast<int>(p1Action) << "\n";
    logFile << "P2 requested: " << static_cast<int>(p2Action) << "\n";
}





std::pair<int, int> GameState::findTank(CellContent tankSymbol) {
    for (int y = 0; y < board.getHeight(); ++y) {
        for (int x = 0; x < board.getWidth(); ++x) {
            if (board.getCell(x, y).content == tankSymbol) {
                return {x, y};
            }
        }
    }
    return {-1, -1}; // Not found
}
std::pair<int, int> GameState::getTank1Position() const {
    return tank1.getPosition();
}
std::pair<int, int> GameState::getTank2Position() const {
    return tank2.getPosition();
}

std::string GameState::render() const {
    if (gameOver) {
        return board.print(tank1.getDirection(), tank2.getDirection()) + "GAME OVER: " + gameResult + "\n";
    }else{
        return  board.print(tank1.getDirection(), tank2.getDirection()) + "\n";
    }
}

std::string GameState::getResult() const {
    return gameOver ? gameResult : "";
}

bool GameState::isGameOver() const {
    return gameOver;
}