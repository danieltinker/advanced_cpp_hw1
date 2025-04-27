#pragma once

#include "Tank.h"

using Position = std::pair<int, int>;

Action decideTank1(
    const std::vector<std::vector<Cell>> &grid,
    Position pos1, Position pos2, int tank1CoolDown, Direction &facing1);
Action decideTank2(
    const std::vector<std::vector<Cell>> &grid,
    Position pos2, Position pos1, Direction &facing2,
    const std::vector<Shell> &shells);

bool hasLineOfSight(
    const std::vector<std::vector<Cell>> &grid,
    Position from, Position to);