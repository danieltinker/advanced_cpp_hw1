#include "Tank.h"

Tank::Tank(int playerId, int x, int y, Direction dir)
    : playerId(playerId), x(x), y(y), direction(dir) {}

int Tank::getPlayerId() const { return playerId; }

std::pair<int, int> Tank::getPosition() const { return {x, y}; }

Direction Tank::getDirection() const { return direction; }

int Tank::getShellCount() const { return shellCount; }

bool Tank::canShoot() const { return shootCooldown == 0 && shellCount > 0; }

bool Tank::isWaitingToMoveBack() const { return backwardRequested && backwardDelay > 0; }

bool Tank::isAlive() const { return alive; }

void Tank::destroy() {
    alive = false;
}

void Tank::updateCooldowns() {
    if (shootCooldown > 0) shootCooldown--;
    if (backwardRequested && backwardDelay > 0) backwardDelay--;
}

void Tank::shoot() {
    if (canShoot()) {
        shootCooldown = 4;
        shellCount--;
    }
}

void Tank::requestBackward() {
    if (!backwardRequested) {
        backwardRequested = true;
        backwardDelay = 2;
    }
}

void Tank::cancelBackwardRequest() {
    if (backwardRequested && backwardDelay > 0) {
        backwardRequested = false;
        backwardDelay = 0;
    }
}

void Tank::confirmBackwardMove() {
    if (backwardRequested && backwardDelay == 0) {
        moveBackward();
        backwardRequested = false;
    }
}

void Tank::rotateLeftEighth() {
    direction = static_cast<Direction>((static_cast<int>(direction) + 7) % 8);
}

void Tank::rotateRightEighth() {
    direction = static_cast<Direction>((static_cast<int>(direction) + 1) % 8);
}

void Tank::rotateLeftQuarter() {
    direction = static_cast<Direction>((static_cast<int>(direction) + 6) % 8);
}

void Tank::rotateRightQuarter() {
    direction = static_cast<Direction>((static_cast<int>(direction) + 2) % 8);
}

void Tank::moveForward() {
    switch (direction) {
        case Direction::U:  y--; break;
        case Direction::UR: x++; y--; break;
        case Direction::R:  x++; break;
        case Direction::DR: x++; y++; break;
        case Direction::D:  y++; break;
        case Direction::DL: x--; y++; break;
        case Direction::L:  x--; break;
        case Direction::UL: x--; y--; break;
    }
}

void Tank::moveBackward() {
    switch (direction) {
        case Direction::U:  y++; break;
        case Direction::UR: x--; y++; break;
        case Direction::R:  x--; break;
        case Direction::DR: x--; y--; break;
        case Direction::D:  y--; break;
        case Direction::DL: x++; y--; break;
        case Direction::L:  x++; break;
        case Direction::UL: x++; y++; break;
    }
}



std::string toString(Direction dir) {
    switch (dir) {
        case Direction::U:  return "↑";
        case Direction::UR: return "↗";
        case Direction::R:  return "→";
        case Direction::DR: return "↘";
        case Direction::D:  return "↓";
        case Direction::DL: return "↙";
        case Direction::L:  return "←";
        case Direction::UL: return "↖";
    }
    return "?"; // Fallback
}
std::string toString(Action ac){
    switch (ac) {
        case Action::MOVE_FORWARD: return "MOVE_FORWARD";
        case Action::MOVE_BACKWARD: return "MOVE_BACKWARD";
        case Action::ROTATE_LEFT_EIGHTH: return "ROTATE_LEFT_EIGHTH";
        case Action::ROTATE_RIGHT_EIGHTH: return "ROTATE_RIGHT_EIGHTH";
        case Action::ROTATE_LEFT_QUARTER: return "ROTATE_LEFT_QUARTER";
        case Action::ROTATE_RIGHT_QUARTER: return "ROTATE_RIGHT_QUARTER";
        case Action::SHOOT: return "SHOOT";
        case Action::NONE: return "NONE";
    }
    return "UNKNOWN_ACTION"; // Fallback
}
