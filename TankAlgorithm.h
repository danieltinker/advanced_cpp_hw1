#pragma once
#include "Tank.h"

using Position = std::pair<int,int>;

Action decideTank1(
    const std::vector<std::vector<Cell>>& grid,
    Position pos2, Direction& facing2,
    int& backwardCooldown2,
    Position pos1);
    Action decideTank2(
        const std::vector<std::vector<Cell>>& grid,
        Position pos2, Direction& facing2,
        int& backwardCooldown2,
        Position pos1,
        const std::vector<Shell>& shells);