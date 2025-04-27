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
    if (!file) {
        throw std::runtime_error("Failed to open board file: " + filePath);
    }

    int tempWidth, tempHeight;
    if (!(file >> tempWidth >> tempHeight)) {
        throw std::runtime_error("Invalid width/height declaration in board file.");
    }
    file.ignore();  // Ignore the rest of the line

    width = tempWidth;
    height = tempHeight;
    grid.resize(height, std::vector<Cell>(width));

    std::ofstream errorLog("input_errors.txt");
    bool hasErrors = false;

    int tank1Count = 0;
    int tank2Count = 0;
    std::string line;

    for (int y = 0; y < height; ++y) {
        if (!std::getline(file, line)) {
            hasErrors = true;
            errorLog << "Warning: Missing row at " << y << ". Filling with EMPTY.\n";
            continue;
        }

        for (int x = 0; x < width; ++x) {
            char ch = (x < (int)line.size()) ? line[x] : ' ';

            switch (ch) {
                case '#':
                    grid[y][x].content = CellContent::WALL;
                    break;
                case '@':
                    grid[y][x].content = CellContent::MINE;
                    break;
                case '1':
                    if (tank1Count == 0) {
                        grid[y][x].content = CellContent::TANK1;
                        tank1Count++;
                    } else {
                        grid[y][x].content = CellContent::EMPTY;
                        hasErrors = true;
                        errorLog << "Warning: Extra Tank 1 ignored at (" << x << "," << y << ").\n";
                    }
                    break;
                case '2':
                    if (tank2Count == 0) {
                        grid[y][x].content = CellContent::TANK2;
                        tank2Count++;
                    } else {
                        grid[y][x].content = CellContent::EMPTY;
                        hasErrors = true;
                        errorLog << "Warning: Extra Tank 2 ignored at (" << x << "," << y << ").\n";
                    }
                    break;
                case ' ':
                    grid[y][x].content = CellContent::EMPTY;
                    break;
                default:
                    grid[y][x].content = CellContent::EMPTY;
                    hasErrors = true;
                    errorLog << "Warning: Unknown character '" << ch << "' treated as EMPTY at (" << x << "," << y << ").\n";
                    break;
            }
        }

        if ((int)line.size() > width) {
            hasErrors = true;
            errorLog << "Warning: Extra characters beyond declared width at row " << y << " ignored.\n";
        }
    }

    // Skip extra rows
    int skippedRows = 0;
    while (std::getline(file, line)) {
        skippedRows++;
    }
    if (skippedRows > 0) {
        hasErrors = true;
        errorLog << "Warning: " << skippedRows << " extra rows ignored beyond declared height.\n";
    }

    errorLog.close();

    if (!hasErrors) {
        std::remove("input_errors.txt");
    }
}


std::string Board::print(Direction dir1,Direction dir2) const {
    std::ostringstream oss;
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            std::string c = " ";
            if (cell.hasShellOverlay) {
                c = '*';  // shell overlay takes precedence
            } else {
            switch (cell.content) {
                case CellContent::WALL:  c = "■"; break;
                case CellContent::MINE:  c = '@'; break;
                case CellContent::TANK1: c = "\033[31m"+toString(dir1)+"\033[0m"; break; // TODO need to use the actual dir once tanks are held in board and not state
                case CellContent::TANK2: c = "\033[34m"+toString(dir2)+"\033[0m";  break;
                case CellContent::SHELL: c = "⋅"; break;
                case CellContent::EMPTY: c = "_"; break;
            }
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
            cell.hasShellOverlay = false;
        }
    }
}

void Board::wrapCoords(int& x, int& y) const {
    x = (x + width) % width;
    y = (y + height) % height;
}
