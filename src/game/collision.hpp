#pragma once
#include "engineDefs.hpp"
#include "vector2.hpp"
#include <vector>
#include <functional>

/*! \file
 * \brief Collision detection functions
 * 
 * This file contains a selection of collision detection functions
 * somewhat useful for a simple 2D game.
 * 
 * One should be aware of some typical naming conventions used here.
 * 
 * *fix*: a Vector2 representing a delta where the object should move to
 * avoid the collision
 * 
 * *ratio*: a floating point value depicting how far along the line the
 * collision occurs. Usually between 0.0 and 1.0.
 * 
 * *found*: a boolean value. True if collision occured.
 */



class TilemapLayer;

class TileMapCollision
{
public:
    bool found = false;
    DefVector2 fix;
    DefVector2 firstNormal;
    DefVector2 firstTile;
    std::vector<DefVector2> allNormals;
    static TileMapCollision FindCollision(DefVector2 center, DefVector2 size, TilemapLayer& map, DefVector2 mapOffset, DefVector2 tileSize,  std::function<DefVector2 (Vector2i,DefVector2)> callback , bool queryAll = false);
    static TileMapCollision FindSweepCollision(DefVector2 c1, DefVector2 c2, DefVector2 size, TilemapLayer& map, DefVector2 mapOffset, DefVector2 tileSize,  std::function<DefVector2 (Vector2i,DefVector2)> callback );
};

class MapTileCollision
{
public:
    bool found = false;
    double ratio = 0;
    DefVector2 fix;
    DefVector2 normal;
    static MapTileCollision FindCollision(DefVector2 center,DefVector2 size,Vector2i, TilemapLayer&, DefVector2);
    static MapTileCollision FindSweepCollision(DefVector2 center1, DefVector2 center2, DefVector2 size,Vector2i, TilemapLayer&,DefVector2);
};

class TileMapLineCollision
{
public:
    bool found = false;
    DefVector2 position;
    DefVector2 normal;
    static TileMapLineCollision FindCollision(DefVector2 p1, DefVector2 p2, TilemapLayer& map, DefVector2 mapOffset, DefVector2 tileSize);
};

class LineIntersection
{
public:
    bool found = false;

    DefVector2 pos;

    double ratio1, ratio2; //collision point measured as p1 + ratio * (p2 - p1)
    static LineIntersection FindCollision(DefVector2 l1p1, DefVector2 l1p2, DefVector2 l2p1, DefVector2 l2p2);
    static LineIntersection FindCollisionRatio(DefVector2 l1p1, DefVector2 l1p2, DefVector2 l2p1, DefVector2 l2p2);
};

class BoxCollision
{
public:
    bool found = false;
    DefVector2 fix;
    static BoxCollision FindCollision(DefVector2 center1, DefVector2 size1, DefVector2 center2, DefVector2 size2);
};



class BoxSweepCollision
{
public:
    bool found = false;
    double ratio = 0;
    DefVector2 o1NewCenter, o2NewCenter;
    DefVector2 o1Normal;
    static BoxSweepCollision FindCollision(DefVector2 o1center1, DefVector2 o1center2, DefVector2 o1size, DefVector2 o2center1, DefVector2 o2center2, DefVector2 o2size, bool o1Static = false, bool o2Static = false);
    static BoxSweepCollision FindCollisionSlide(DefVector2 o1center1, DefVector2 o1center2, DefVector2 o1size, DefVector2 o2center1, DefVector2 o2center2, DefVector2 o2size, bool o1Static, bool o2Static);
};


class BoxLineCollision
{
public:
    bool found = false;
    bool inside = false;
    DefVector2 normal;
    double ratio; //similar ratio as in lineintersection
    DefVector2 point;
    static BoxLineCollision FindCollision(DefVector2 center, DefVector2 size, DefVector2 p1, DefVector2 p2);
    static BoxLineCollision FindCollisionRatio(DefVector2 center, DefVector2 size, DefVector2 p1, DefVector2 p2);
    static BoxLineCollision FindCollisionPoint(DefVector2 center, DefVector2 size, DefVector2 p1, DefVector2 p2);
};
