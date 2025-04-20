// ===============================
// File: GameState.cpp
// Implementation of GameState class and reactive tank logic
// ===============================

#include "GameState.h"
#include <map>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include <cmath>

// --- Global Game State Flags ---
int emptyAmmoSteps = 0;
bool gameOver = false;
std::string gameResult;
std::ofstream logFile("game_output.txt");


GameState::GameState(Board& board): board(board),
      tank1([&] { auto [x1, y1] = findTank(CellContent::TANK1); return Tank(1, x1, y1, Direction::L); }()),
      tank2([&] { auto [x2, y2] = findTank(CellContent::TANK2); return Tank(2, x2, y2, Direction::R); }()) {}



// --- Simple Reactive AI for Tank 2 ---
Action simpleReactive(const Tank& tank, const std::vector<Shell>& shells, const Board& board) {
    auto [tx, ty] = tank.getPosition();
    for (const auto& shell : shells) {
        int dx = std::abs(shell.x - tx);
        int dy = std::abs(shell.y - ty);
        if (dx <= 2 && dy <= 2) {
            return Action::ROTATE_LEFT_QUARTER; // dodge reflex
        }
    }
    if (tank.canShoot()) return Action::SHOOT;
    return Action::MOVE_FORWARD;
}

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


// --- Main Game Tick Step ---
void GameState::step(Action p1Action, Action p2Action) {
    if (gameOver) return;

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

    p2Action = simpleReactive(tank2, shells, board);
    logFile << "P1: " << static_cast<int>(p1Action) << ", P2: " << static_cast<int>(p2Action) << "\n";

    tank1.updateCooldowns();
    tank2.updateCooldowns();

    applyAction(tank1, p1Action);
    applyAction(tank2, p2Action);

    tank1.confirmBackwardMove();
    tank2.confirmBackwardMove();

    board.clearTankMarks();
    x1 = tank1.getPosition().first;
    y1 = tank1.getPosition().second;
    x2 = tank2.getPosition().first;
    y2 = tank2.getPosition().second;
    if (tank1.isAlive()) board.setCell(x1, y1, CellContent::TANK1);
    if (tank2.isAlive()) board.setCell(x2, y2, CellContent::TANK2);

    board.clearShellMarks();
    std::map<std::pair<int, int>, std::vector<size_t>> positionMap;
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
        positionMap[{shells[i].x, shells[i].y}].push_back(i);
    }

    std::set<size_t> toRemove;
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
            if (board.grid[y][x].wallHits >= 2) board.setCell(x, y, CellContent::EMPTY);
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

    std::vector<Shell> remaining;
    for (size_t i = 0; i < shells.size(); ++i) {
        if (toRemove.find(i) == toRemove.end()) {
            remaining.push_back(shells[i]);
            board.setCell(shells[i].x, shells[i].y, CellContent::SHELL);
        }
    }
    shells = std::move(remaining);

    if (p1Action == Action::SHOOT && tank1.canShoot()) {
        tank1.shoot();
        auto [sx, sy] = tank1.getPosition();
        shells.push_back({sx, sy, tank1.getDirection()});
    }
    if (p2Action == Action::SHOOT && tank2.canShoot()) {
        tank2.shoot();
        auto [sx, sy] = tank2.getPosition();
        shells.push_back({sx, sy, tank2.getDirection()});
    }

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

Action GameState::chaseTarget(int srcX, int srcY, int tgtX, int tgtY) {
    std::queue<std::pair<int, int>> q;
    std::vector<std::vector<bool>> visited(board.getHeight(), std::vector<bool>(board.getWidth(), false));
    std::vector<std::vector<std::pair<int, int>>> parent(board.getHeight(), std::vector<std::pair<int, int>>(board.getWidth(), {-1, -1}));

    q.push({srcX, srcY});
    visited[srcY][srcX] = true;

    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();
        if (x == tgtX && y == tgtY) break;

        for (int d = 0; d < 4; ++d) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (nx >= 0 && ny >= 0 && nx < board.getWidth() && ny < board.getHeight()) {
                if (!visited[ny][nx] && board.getCell(nx, ny).content == CellContent::EMPTY) {
                    visited[ny][nx] = true;
                    parent[ny][nx] = {x, y};
                    q.push({nx, ny});
                }
            }
        }
    }

    std::pair<int, int> cur = {tgtX, tgtY};
    while (parent[cur.second][cur.first] != std::make_pair(srcX, srcY)) {
        cur = parent[cur.second][cur.first];
        if (cur == std::make_pair(-1, -1)) return Action::NONE;
    }
    int dx1 = cur.first - srcX;
    int dy1 = cur.second - srcY;
    if (dx1 == 0 && dy1 == -1) return Action::MOVE_FORWARD;
    if (dx1 == 0 && dy1 == 1) return Action::MOVE_BACKWARD;
    if (dx1 == 1 && dy1 == 0) return Action::ROTATE_RIGHT_EIGHTH;
    if (dx1 == -1 && dy1 == 0) return Action::ROTATE_LEFT_EIGHTH;
    return Action::NONE;
}

void GameState::render() const {
    board.print();
    if (gameOver) {
        std::cout << "GAME OVER: " << gameResult << "\n";
    }
}

std::string GameState::getResult() const {
    return gameOver ? gameResult : "";
}

bool GameState::isGameOver() const {
    return gameOver;
}