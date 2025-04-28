#pragma once

#include <string>
#include <utility>

enum class Direction {
    U, UR, R, DR, D, DL, L, UL
};


enum class Action {
    MOVE_FORWARD,
    MOVE_BACKWARD,
    ROTATE_LEFT_EIGHTH,
    ROTATE_RIGHT_EIGHTH,
    ROTATE_LEFT_QUARTER,
    ROTATE_RIGHT_QUARTER,
    SHOOT,
    NONE
};


class Tank {
    friend int main(int argc, char* argv[]); 
public:
    Tank(int playerId, int x, int y, Direction dir);

    int getPlayerId() const;
    std::pair<int, int> getPosition() const;
    Direction getDirection() const;
    int getShellCount() const;
    bool canShoot() const;
    bool isWaitingToMoveBack() const;

    // Called every game step
    void updateCooldowns();

    void requestBackward();
    void cancelBackwardRequest();
    void confirmBackwardMove();

    void shoot();

    void rotateLeftEighth();
    void rotateRightEighth();
    void rotateLeftQuarter();
    void rotateRightQuarter();

    void moveForward();
    void moveBackward(); // called by GameManager after cooldown

    void destroy();
    bool isAlive() const;

private:
    int playerId;
    int x, y;
    Direction direction;
    int shellCount = 16;

    int shootCooldown = 0;
    bool alive = true;

    // Backward move tracking
    int backwardDelay = 0;
    bool backwardRequested = false;
};


std::string toString(Direction dir);
std::string toString(Action ac);
