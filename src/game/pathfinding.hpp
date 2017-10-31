#pragma once
#include "engineDefs.hpp"
#include <list>
class GameMapHandler;

class Pathfinder
{
protected:
    GameMapHandler* mapHandler;
    DefVector2 begin;
    DefVector2 end;
    DefVector2 size = {0,0};
    bool direct = false;
    bool checkForSize = false;
    void pathfind();
public:
    int maximumDepth = 6000;
    std::list<DefVector2> points;
    Pathfinder(GameMapHandler* map) : mapHandler(map) {};
    void setEnd(DefVector2);
    void setBegin(DefVector2);
    void setPathSize(DefVector2);
    void update();
    bool isDirect();
    DefVector2 getTargetPosition();

};
