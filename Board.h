#pragma once

#include <vector>
#include <string>
#include "Tank.h"
enum class CellContent {
    EMPTY,
    WALL,
    MINE,
    TANK1,
    TANK2,
    SHELL
};

struct Cell {
    CellContent content = CellContent::EMPTY;
    int wallHits = 0;
};

class Board {
public:
    friend int main(int argc, char* argv[]); // TODO remove
    
    Board(const std::string& filePath);
    std::string print(Direction dir1,Direction dir2) const;
    int getWidth() const;
    int getHeight() const;
    Cell getCell(int x, int y) const;
    void setCell(int x, int y, CellContent content);
    void clearTankMarks();
    void clearShellMarks();
    void wrapCoords(int& x, int& y) const;

    std::vector<std::vector<Cell>> grid;

private:
    int width = 0, height = 0;
    void parseBoardFile(const std::string& filePath);
};
