#include "pathfinding.hpp"

#include "log.hpp"
#include <cassert>
#include <set>

/*

//Pathfinding

class PathfindingCell
{
    int value;
public:
    const Vector2i cell;
    const Vector2i target;
    const int iteration;
    const Vector2i master;


    PathfindingCell(Vector2i a, Vector2i b, int c, Vector2i d) : cell(a), target(b), iteration(c), master(d)
    {
        value = (b-a).length()+iteration;
    }

    int getValue() const
    {
        return value;
    }
};
bool operator > (const PathfindingCell a,const PathfindingCell b) {return (a.getValue()>b.getValue());};
bool operator < (const PathfindingCell a, const PathfindingCell b) {return (a.getValue()<b.getValue());};
bool operator == (const PathfindingCell a, const PathfindingCell b) {return (a.getValue()==b.getValue());};

class ClosedPathfindingCell
{
public:
    const Vector2i cell;
    const Vector2i master;
    ClosedPathfindingCell(Vector2i a, Vector2i b) : cell(a), master(b) {};
    ClosedPathfindingCell(PathfindingCell& b) : cell(b.cell), master(b.master) {};
};
bool operator > (const ClosedPathfindingCell a, const ClosedPathfindingCell b)
{
    if (a.cell.x == b.cell.x)
        return (a.cell.y>b.cell.y);
    return a.cell.x > b.cell.x;
}
bool operator < (const ClosedPathfindingCell a, const ClosedPathfindingCell b)  {return (b > a);};
bool operator == (const ClosedPathfindingCell a, const ClosedPathfindingCell b) {return (a.cell == b.cell);};

void Pathfinder::pathfind()
{
    Vector2i ec = mapHandler->getTilePosition(begin);
    Vector2i bc = mapHandler->getTilePosition(end);
    points.clear();

    if (!mapHandler->getMap()->getTileExists(ec))
        return;
    if (!mapHandler->getMap()->getTileExists(bc))
        return;

    if (mapHandler->getTileIsBlocking(ec))
        return;
    if (mapHandler->getTileIsBlocking(bc))
        return;
    std::multiset<PathfindingCell> ocells;
    std::set<ClosedPathfindingCell> ccells;

    ocells.insert(PathfindingCell(bc,ec,0,bc));
    ccells.insert(ClosedPathfindingCell(bc,bc));
    bool found = false;
    PathfindingCell* fCell = nullptr;

    int depth = 0;

    while (ocells.size())
    {
        depth++;
        if (depth > maximumDepth)
            return;
        auto it = ocells.begin();

        PathfindingCell cell = *it;

        if (cell.cell == ec)
        {
            found = true;
            fCell = &cell;
            break;
        }


        Vector2i dirs[] = {{0,1},{1,0},{0,-1},{-1,0}};

        for (int i = 0; i < 4; i++)
        {
            Vector2i p = cell.cell+dirs[i];

            if (ccells.count(ClosedPathfindingCell(p,cell.cell)))
            {
                continue;
            }
            if (!mapHandler->getMap()->getTileExists(p))
                continue;
            if (mapHandler->getTileIsBlocking(p))
                continue;

            ccells.insert(ClosedPathfindingCell(p,cell.cell));
            ocells.insert(PathfindingCell(p,ec,cell.iteration+1,cell.cell));
        }

        ocells.erase(it);
    }

    if (found == true && fCell)
    {
        Vector2i addi = fCell->master;

        while (true)
        {
            DefVector2 addf = (addi * mapHandler->getTileSize()) + mapHandler->getTileSize()/2;
            points.push_back(addf);

            ClosedPathfindingCell c(addi,addi);

            auto it = ccells.find(c);
            if (it == ccells.end())
                assert(false && "no master cell found");

            if (addi == (*it).master)
                break;

            addi = (*it).master;
        }
        points.push_back(end);
    }

}

void Pathfinder::setBegin(DefVector2 b)
{
    begin = b;
}


void Pathfinder::setPathSize(DefVector2 s)
{
    size = s/2;
    if (size.length() > 4)
        checkForSize = true;
}

void Pathfinder::setEnd(DefVector2 e)
{
    end = e;
}

void Pathfinder::update()
{

    points.clear();
    points.push_back(end);
    direct = false;
    if (!checkForSize)
    {
        if (!mapHandler->getLineCollision(begin, end))
        {
            direct = true;
            return;
        }
    }
    else
    {
        direct = false;
        if (!mapHandler->getLineCollision(begin-size, end))
            if (!mapHandler->getLineCollision(begin-size, end))
                if (!mapHandler->getLineCollision(begin-DefVector2(size.x,-size.y), end))
                    if (!mapHandler->getLineCollision(begin+DefVector2(size.x,-size.y), end))
                    {
                        direct = true;
                        return;
                    }
    }

    pathfind();


}

bool Pathfinder::isDirect()
{
    return direct;
}


DefVector2 Pathfinder::getTargetPosition()
{
    if (direct)
        return end;
    return end;
}


*/
