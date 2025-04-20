#include <vector>
#include <queue>
#include <cmath>
#include <limits>
#include <utility>
#include "Tank.h"
#include "Board.h"
#include <algorithm>


using Position = std::pair<int,int>;

// Offsets for 8 directions
static const Position dirOffsets[8] = {
    { 0,-1}, { 1,-1}, { 1, 0}, { 1, 1},
    { 0, 1}, {-1, 1}, {-1, 0}, {-1,-1}
};

// Check in-bounds
inline bool inBounds(const std::vector<std::vector<Cell>>& grid, const Position& p) {
    return p.second >= 0 && p.second < (int)grid.size()
        && p.first  >= 0 && p.first  < (int)grid[0].size();
}

// Convert two positions into one of 8 directions
Direction directionTo(const Position& from, const Position& to) {
    int dx = to.first - from.first;
    int dy = to.second - from.second;
    dx = (dx>0) - (dx<0);
    dy = (dy>0) - (dy<0);
    for(int d=0; d<8; ++d) {
        if(dirOffsets[d].first == dx && dirOffsets[d].second == dy)
            return static_cast<Direction>(d);
    }
    return Direction::U;
}

// Rotate from current towards target by smallest step
Action rotateTowards(Direction current, Direction target) {
    int cur = static_cast<int>(current);
    int tgt = static_cast<int>(target);
    int diff = (tgt - cur + 8) % 8;
    if(diff == 0) return Action::NONE;
    if(diff <= 4) {
        return diff == 4 ? Action::ROTATE_RIGHT_QUARTER : Action::ROTATE_RIGHT_EIGHTH;
    } else {
        int ld = (cur - tgt + 8) % 8;
        return ld == 4 ? Action::ROTATE_LEFT_QUARTER : Action::ROTATE_LEFT_EIGHTH;
    }
}

// A* pathfinding avoiding walls and mines
std::vector<Position> findPath(
    const std::vector<std::vector<Cell>>& grid,
    Position start, Position goal)
{
    if(!inBounds(grid,start) || !inBounds(grid,goal)) return {};
    int H = grid.size(), W = grid[0].size();
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<std::vector<double>> gScore(H, std::vector<double>(W, INF));
    std::vector<std::vector<Position>> parent(H, std::vector<Position>(W, {-1,-1}));

    struct Node { double f; Position pos; };
    auto cmp = [](auto &a, auto &b){ return a.f > b.f; };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> open(cmp);

    auto heur = [&](const Position &p){
        double dx = goal.first - p.first;
        double dy = goal.second - p.second;
        return std::hypot(dx, dy);
    };

    gScore[start.second][start.first] = 0;
    open.push({heur(start), start});

    while(!open.empty()) {
        auto [f, cur] = open.top(); open.pop();
        if(cur == goal) break;
        // Skip if we've found a better path already
        if(f > gScore[cur.second][cur.first] + heur(cur)) continue;
        for(int d=0; d<8; ++d) {
            Position nb{cur.first + dirOffsets[d].first,
                        cur.second + dirOffsets[d].second};
            if(!inBounds(grid, nb)) continue;
            auto c = grid[nb.second][nb.first].content;
            if(c == CellContent::WALL || c == CellContent::MINE) continue;
            double cost = (d % 2 == 0 ? 1.0 : 1.414);
            double tent = gScore[cur.second][cur.first] + cost;
            if(tent < gScore[nb.second][nb.first]) {
                gScore[nb.second][nb.first] = tent;
                parent[nb.second][nb.first] = cur;
                open.push({tent + heur(nb), nb});
            }
        }
    }
    if(parent[goal.second][goal.first].first < 0) return {};

    std::vector<Position> path;
    for(Position at = goal; at != start; at = parent[at.second][at.first]) {
        path.push_back(at);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

// Line-of-sight check with bounds
bool hasLineOfSight(
    const std::vector<std::vector<Cell>>& grid,
    Position from, Position to)
{
    int dx = to.first - from.first;
    int dy = to.second - from.second;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if(steps == 0) return true;
    double sx = dx / static_cast<double>(steps);
    double sy = dy / static_cast<double>(steps);
    double x = from.first, y = from.second;
    for(int i=0; i<steps; ++i) {
        x += sx; y += sy;
        int ix = std::round(x);
        int iy = std::round(y);
        Position p{ix, iy};
        if(!inBounds(grid, p) || grid[iy][ix].content == CellContent::WALL)
            return false;
    }
    return true;
}

// --- Tank1 AI ---
Action decideTank1(
    const std::vector<std::vector<Cell>>& grid,
    Position pos1, Direction& facing1,
    int& shootCooldown1,
    Position pos2)
{
    if(shootCooldown1 > 0) --shootCooldown1;
    // Try shoot if possible
    if(shootCooldown1 == 0 && hasLineOfSight(grid, pos1, pos2)) {
        Direction toT = directionTo(pos1, pos2);
        if(facing1 == toT) {
            shootCooldown1 = 4;
            return Action::SHOOT;
        }
        return rotateTowards(facing1, toT);
    }
    // Move along A* path
    auto path = findPath(grid, pos1, pos2);
    if(path.size() >= 2) {
        Position next = path[1];
        Direction want = directionTo(pos1, next);
        if(facing1 != want)
            return rotateTowards(facing1, want);
        // next cell guaranteed free
        return Action::MOVE_FORWARD;
    }
    return Action::NONE;
}

// --- Tank2 AI ---
Action decideTank2(
    const std::vector<std::vector<Cell>>& grid,
    Position pos2, Direction& facing2,
    int& backwardCooldown2,
    Position pos1)
{
    if(backwardCooldown2 > 0) --backwardCooldown2;
    // Determine away direction
    Direction from1 = directionTo(pos1, pos2);
    int oppDir = (static_cast<int>(from1) + 4) % 8;
    Direction want = static_cast<Direction>(oppDir);

    if(facing2 == want) {
        // Attempt backward step (i.e., opposite of facing)
        Position step{-dirOffsets[static_cast<int>(facing2)].first,
                      -dirOffsets[static_cast<int>(facing2)].second};
        Position target{pos2.first + step.first, pos2.second + step.second};
        if(backwardCooldown2 == 0 && inBounds(grid, target)) {
            auto c = grid[target.second][target.first].content;
            if(c != CellContent::WALL && c != CellContent::MINE) {
                backwardCooldown2 = 2;
                return Action::MOVE_BACKWARD;
            }
        }
    }
    // Otherwise rotate away from pursuer
    return rotateTowards(facing2, want);
}

// Usage: maintain each tank's state (position, facing, cooldowns) and call decideTank1/2 per tick.
