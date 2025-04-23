#include "Board.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "Tank.h"
#include <sstream>


Board::Board(const std::string& filePath) {
    parseBoardFile(filePath);
}

void Board::parseBoardFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) throw std::runtime_error("Failed to open board file.");

    file >> width >> height;
    file.ignore();
    grid.resize(height, std::vector<Cell>(width));

    std::string line;
    std::getline(file, line); // ignoring the first size defining line
    for (int y = 0; y < height && std::getline(file, line); ++y) {
        for (int x = 0; x < width && x < (int)line.size(); ++x) {
            switch (line[x]) {
                case '#': grid[y][x].content = CellContent::WALL; break;
                case '@': grid[y][x].content = CellContent::MINE; break;
                case '1': grid[y][x].content = CellContent::TANK1; break;
                case '2': grid[y][x].content = CellContent::TANK2; break;
                case ' ': grid[y][x].content = CellContent::EMPTY; break;
                default:  grid[y][x].content = CellContent::EMPTY; break;
            }
        }
    }
}

std::string Board::print(Direction dir1,Direction dir2) const {
    std::ostringstream oss;
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            std::string c = " ";
            switch (cell.content) {
                case CellContent::WALL:  c = "■"; break;
                case CellContent::MINE:  c = '@'; break;
                case CellContent::TANK1: c = "\033[31m"+toString(dir1)+"\033[0m"; break; // TODO need to use the actual dir once tanks are held in board and not state
                case CellContent::TANK2: c = "\033[34m"+toString(dir2)+"\033[0m";  break;
                case CellContent::SHELL: c = "⋅"; break;
                case CellContent::EMPTY: c = "_"; break;
            }
            oss << c;
        }
        oss << '\n';
    }  
    return oss.str();
}

int Board::getWidth() const { return width; }
int Board::getHeight() const { return height; }

Cell Board::getCell(int x, int y) const {
    return grid[y][x];
}

void Board::setCell(int x, int y, CellContent content) {
    grid[y][x].content = content;
}

void Board::clearTankMarks() {
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell.content == CellContent::TANK1 || cell.content == CellContent::TANK2)
                cell.content = CellContent::EMPTY;
        }
    }
}

void Board::clearShellMarks() {
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell.content == CellContent::SHELL)
                cell.content = CellContent::EMPTY;
        }
    }
}

void Board::wrapCoords(int& x, int& y) const {
    x = (x + width) % width;
    y = (y + height) % height;
}
