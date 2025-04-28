#include <vector>
#include <queue>
#include <cmath>
#include <limits>
#include <utility>
#include "Tank.h"
#include "Board.h"
#include <algorithm>
#include "GameState.h"

using Position = std::pair<int, int>;

// Offsets for 8 directions
static const Position dirOffsets[8] = {
    {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};

// Check in-bounds
inline bool inBounds(const std::vector<std::vector<Cell>> &grid, const Position &p)
{
    return p.second >= 0 && p.second < (int)grid.size() && p.first >= 0 && p.first < (int)grid[0].size();
}

// Convert two positions into one of 8 directions
Direction directionTo(const Position &from, const Position &to)
{
    int dx = to.first - from.first;
    int dy = to.second - from.second;
    dx = (dx > 0) - (dx < 0);
    dy = (dy > 0) - (dy < 0);
    for (int d = 0; d < 8; ++d)
    {
        if (dirOffsets[d].first == dx && dirOffsets[d].second == dy)
            return static_cast<Direction>(d);
    }
    return Direction::U;
}

// Rotate from current towards target by smallest step
Action rotateTowards(Direction current, Direction target)
{
    int cur = static_cast<int>(current);
    int tgt = static_cast<int>(target);
    int diff = (tgt - cur + 8) % 8;
    if (diff == 0)
        return Action::NONE;
    if (diff <= 4)
    {
        return diff == 4 ? Action::ROTATE_RIGHT_QUARTER : Action::ROTATE_RIGHT_EIGHTH;
    }
    else
    {
        int ld = (cur - tgt + 8) % 8;
        return ld == 4 ? Action::ROTATE_LEFT_QUARTER : Action::ROTATE_LEFT_EIGHTH;
    }
}

// A* pathfinding avoiding walls and mines
std::vector<Position> findPath(
    const std::vector<std::vector<Cell>> &grid,
    Position start, Position goal)
{
    if (!inBounds(grid, start) || !inBounds(grid, goal))
        return {};
    int H = grid.size(), W = grid[0].size();
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<std::vector<double>> gScore(H, std::vector<double>(W, INF));
    std::vector<std::vector<Position>> parent(H, std::vector<Position>(W, {-1, -1}));

    struct Node
    {
        double f;
        Position pos;
    };
    auto cmp = [](auto &a, auto &b)
    { return a.f > b.f; };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> open(cmp);

    auto heur = [&](const Position &p)
    {
        double dx = goal.first - p.first;
        double dy = goal.second - p.second;
        return std::hypot(dx, dy);
    };

    gScore[start.second][start.first] = 0;
    open.push({heur(start), start});

    while (!open.empty())
    {
        auto [f, cur] = open.top();
        open.pop();
        if (cur == goal)
            break;
        if (f > gScore[cur.second][cur.first] + heur(cur))
            continue;
        for (int d = 0; d < 8; ++d)
        {
            Position nb{cur.first + dirOffsets[d].first,
                        cur.second + dirOffsets[d].second};
            if (!inBounds(grid, nb))
                continue;
            auto c = grid[nb.second][nb.first].content;
            if (c == CellContent::WALL || c == CellContent::MINE)
                continue;
            double cost = (d % 2 == 0 ? 1.0 : 1.414);
            double tent = gScore[cur.second][cur.first] + cost;
            if (tent < gScore[nb.second][nb.first])
            {
                gScore[nb.second][nb.first] = tent;
                parent[nb.second][nb.first] = cur;
                open.push({tent + heur(nb), nb});
            }
        }
    }

    if (parent[goal.second][goal.first].first < 0)
        return {};

    std::vector<Position> path;
    for (Position at = goal; at != start; at = parent[at.second][at.first])
    {
        path.push_back(at);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

// Line-of-sight check with bounds
bool hasLineOfSight(const std::vector<std::vector<Cell>> &grid,
                    Position from, Position to)
{
    if (from == to)
        return true; // Shouldnt happen

    int dx = to.first - from.first;
    int dy = to.second - from.second;

    for (Position dir : dirOffsets) // unit directions
    {
        int cross = dx * dir.second - dy * dir.first; // 2-D cross product
        int dot = dx * dir.first + dy * dir.second;   // orientation test

        if (cross == 0 && dot > 0) // on the same ray
        {
            int steps = std::max(std::abs(dx), std::abs(dy)); // |longest axis|

            for (int s = 1; s <= steps; ++s)
            {
                Position p{from.first + dir.first * s,
                           from.second + dir.second * s};

                if (!inBounds(grid, p) ||
                    grid[p.second][p.first].content == CellContent::WALL)
                    return false; // blocked

                if (p == to)
                    return true; // reached target
            }
            return false; // ran out of steps without hitting 'to'
        }
    }
    return false; // no matching direction found
}


Action decideTank1(
    const std::vector<std::vector<Cell>> &grid,
    Position pos1, Position pos2,
    int tank1CoolDown, Direction &facing1)
{
    if (tank1CoolDown == 0 && hasLineOfSight(grid, pos1, pos2)) {
        Direction toT = directionTo(pos1, pos2);
        if (facing1 == toT)       return Action::SHOOT;
        else                      return rotateTowards(facing1, toT);
    }

    static std::vector<Position> cachedPath;
    static int tick = 0;                     // counts calls to this function to modulate pathfinding calls

    // Recompute only (a) on the first call, (b) every 4th call, or (c) if the goal changed
    if (cachedPath.empty() || tick % 4 == 0 || cachedPath.back() != pos2) {
        cachedPath = findPath(grid, pos1, pos2);
        tick = 0;                            // restart the counter after a fresh path
    }
    ++tick;

    // remove already-visited nodes so that cachedPath[0] == pos1 
    while (!cachedPath.empty() && cachedPath.front() == pos1)
        cachedPath.erase(cachedPath.begin());

    if (cachedPath.size() >= 1) {            // ≥1 because we just stripped pos1
        Position next = cachedPath.front(); 
        Direction want = directionTo(pos1, next);

        if (facing1 != want)
            return rotateTowards(facing1, want);

        // if the cell immediately ahead already holds Tank 2, stop
        bool willCollide =
            pos1.first  + dirOffsets[static_cast<int>(facing1)].first  == pos2.first &&
            pos1.second + dirOffsets[static_cast<int>(facing1)].second == pos2.second;

        return willCollide ? Action::NONE : Action::MOVE_FORWARD;
    }

    return Action::NONE;
}


Action decideTank2(
    const std::vector<std::vector<Cell>> &grid,
    Position pos2, Position pos1, Direction &facing2,
    const std::vector<Shell> &shells)
{

    // Shell dodging
    for (const auto &s : shells)
    {
        Position sp{s.x, s.y};
        for (int i = 1; i <= 5; ++i)
        {
            Position pred{sp.first + dirOffsets[static_cast<int>(s.dir)].first * i,
                          sp.second + dirOffsets[static_cast<int>(s.dir)].second * i};
            if (pred == pos2)
            {
                // Try sidestepping
                int right = (static_cast<int>(facing2) + 2) % 8;
                int left = (static_cast<int>(facing2) + 6) % 8;
                Position rpos{pos2.first + dirOffsets[right].first, pos2.second + dirOffsets[right].second};
                Position lpos{pos2.first + dirOffsets[left].first, pos2.second + dirOffsets[left].second};

                if (inBounds(grid, rpos) && grid[rpos.second][rpos.first].content == CellContent::EMPTY)
                {
                    if (facing2 != static_cast<Direction>(right))
                        return rotateTowards(facing2, static_cast<Direction>(right));
                    return Action::MOVE_FORWARD;
                }
                if (inBounds(grid, lpos) && grid[lpos.second][lpos.first].content == CellContent::EMPTY)
                {
                    if (facing2 != static_cast<Direction>(left))
                        return rotateTowards(facing2, static_cast<Direction>(left));
                    return Action::MOVE_FORWARD;
                }
                return Action::ROTATE_LEFT_EIGHTH;
            }
        }
    }

    Direction from1 = directionTo(pos1, pos2);
    int dirIndex = static_cast<int>(from1);
    int tangentDir = (dirIndex + 2) % 8;
    Direction want = static_cast<Direction>(tangentDir);

    if (facing2 != want)
        return rotateTowards(facing2, want);

    Position step = dirOffsets[static_cast<int>(facing2)];
    Position target{pos2.first + step.first, pos2.second + step.second};
    if (inBounds(grid, target))
    {
        auto c = grid[target.second][target.first].content;
        if (c == CellContent::EMPTY)
            return Action::MOVE_FORWARD;
    }

    return Action::NONE;
}

