
#include <algorithm>
#include "collision.hpp"
#include "gridLine.hpp"
#include "game/tilemap.hpp"


TileMapLineCollision TileMapLineCollision::FindCollision(DefVector2 p1, DefVector2 p2, TilemapLayer& map, DefVector2 mapOffset, DefVector2 tileSize)
{
    DefVector2 minp;
    DefVector2 maxp;
    minp.x = std::min(p1.x,p2.x);
    minp.y = std::min(p1.y,p2.y);
    maxp.x = std::max(p1.x,p2.x);
    maxp.y = std::max(p1.y,p2.y);

    TileMapLineCollision tmlc;
    tmlc.found = false;
    
    Vector2i minb {-1, -1};
    Vector2i maxb = map.getSize();
    
    auto inbounds = [&](const Vector2i& c)
    {
        return c.inside(minb, maxb);
    };
    
    Vector2i lastp = Vector2i(p1/tileSize);
    GridLine::gridIterateCellsOnLineScale(tileSize, p1, p2,[&](const Vector2i& curp)
    {
        if (!inbounds(curp))
            return false;
        if (map.getTile(curp) == 1)
        {
            tmlc.found = true;
            DefVector2 min, max;
            if (lastp.x != curp.x)
            {
                min.y = minp.y-1;
                max.y = maxp.y+1;

                tmlc.normal.x = 1;
                tmlc.normal.y = 0;

                max.x = lastp.x*tileSize.x;
                if (lastp.x < curp.x)
                {
                    max.x += tileSize.x;
                    tmlc.normal.x = -1;
                }

                min.x = max.x;
            }
            else
            {
                min.x = minp.x-1;
                max.x = maxp.x+1;

                tmlc.normal.x = 0;
                tmlc.normal.y = 1;

                max.y = lastp.y*tileSize.y;
                if (lastp.y < curp.y)
                {
                    max.y += tileSize.y;
                    tmlc.normal.y = -1;
                }

                min.y = max.y;
            }


            LineIntersection li = LineIntersection::FindCollision(p1,p2,min,max);

            tmlc.position = li.pos;

            return false;
        }
        lastp = curp;
        return true;
    });
    return tmlc;
}

TileMapCollision TileMapCollision::FindCollision(DefVector2 center, DefVector2 size, TilemapLayer& map, DefVector2 mapOffset, DefVector2 tileSize, std::function<DefVector2 (Vector2i,DefVector2)> callback, bool queryAll)
{
    
    
    Vector2i minb {-1, -1};
    Vector2i maxb = map.getSize();
    
    auto inbounds = [&](const Vector2i& c)
    {
        return c.inside(minb, maxb);
    };
    
    auto isblocking = [](const int i)
    {
        return i == 1;  
    };
    
    auto iscallback = [](const int i)
    {
        return i >= 80;  
    };
    
    DefVector2 oCenter = center;
    TileMapCollision result;
    DefVector2 halfSize = size/2;
    Vector2i centerCell = (center-mapOffset)/tileSize;
    Vector2i topLeftCell;
    Vector2i bottomRightCell;

    DefVector2 tls = (center-halfSize-mapOffset)/tileSize;
    topLeftCell = tls;
    if (tls.x < 0)
        topLeftCell.x--;
    if (tls.y < 0)
        topLeftCell.y--;

    tls = (center+halfSize-mapOffset)/tileSize;

    bottomRightCell = tls;
    if (tls.x < 0)
        bottomRightCell.x--;
    if (tls.y < 0)
        bottomRightCell.y--;

    if (topLeftCell.x > bottomRightCell.x)
        topLeftCell.x = bottomRightCell.x;
    if (topLeftCell.y > bottomRightCell.y)
        topLeftCell.y = bottomRightCell.y;

    if (centerCell.x > bottomRightCell.x)
        centerCell.x = bottomRightCell.x;
    if (centerCell.y > bottomRightCell.y)
        centerCell.y = bottomRightCell.y;

    if (centerCell.x < topLeftCell.x)
        centerCell.x = topLeftCell.x;
    if (centerCell.y < topLeftCell.y)
        centerCell.y = topLeftCell.y;
/*
    if (topLeftCell.x >= map->getDimensions().x || topLeftCell.y >= map->getDimensions().y)
        return result;
    if (bottomRightCell.x < 0 || bottomRightCell.y < 0)
        return result;
    */

    

    int xranges[] = {centerCell.x,1,bottomRightCell.x+1,centerCell.x-1,-1,topLeftCell.x-1};

    int yranges[] = {centerCell.y,-1,topLeftCell.y-1,centerCell.y+1,1,bottomRightCell.y+1};

    bool first = true;
    int rangeCombinations[] = {0,0, 0,1, 1,0, 1,1};
    for (int i = 0; i < 4; i+=1)
    {
        int xrangeB =   xranges[rangeCombinations[i*2]*3];
        int xrangeAdd = xranges[rangeCombinations[i*2]*3+1];
        int xrangeE =   xranges[rangeCombinations[i*2]*3+2];

        int yrangeB =   yranges[rangeCombinations[i*2+1]*3];
        int yrangeAdd = yranges[rangeCombinations[i*2+1]*3+1];
        int yrangeE =   yranges[rangeCombinations[i*2+1]*3+2];

        for (int x = xrangeB; x != xrangeE; x+=xrangeAdd)
        for (int y = yrangeB; y != yrangeE; y+=yrangeAdd)
        {
            DefVector2 tcell = Vector2i(x,y);
            /*
            if (tcell.x < 0 || tcell.x >= map->getDimensions().x)
                continue;
            if (tcell.y < 0 || tcell.y >= map->getDimensions().y)
                continue;
            */

            auto t = map.getTile({x,y});

            if (inbounds({x,y}))
            if (!(isblocking(t) || iscallback(t)))
                continue;

            //if (t->triggerCallback == true)

            if (queryAll)
            {
                MapTileCollision coll = MapTileCollision::FindCollision(oCenter,size,tcell,map,tileSize);
                if (coll.found)
                {
                    result.allNormals.push_back(coll.fix);
                }
            }

            //BoxRampCollision coll = BoxRampCollision::FindCollisionHInverted(center,size,DefVector2(tcell*tileSize)+mapOffset+tileSize/2,tileSize);
            MapTileCollision coll = MapTileCollision::FindCollision(center,size,tcell,map,tileSize);
            //BoxCollision coll = BoxCollision::FindCollision(center,size,DefVector2(tcell*tileSize)+mapOffset+tileSize/2,tileSize);



            result.found =result.found||coll.found;
            if (coll.found)
            {

                if (inbounds({x,y}))
                if (callback)
                {
                    if (iscallback(t))
                    {
                        coll.fix = callback(Vector2i(x,y),coll.fix);
                        if (coll.fix.x == 0 && coll.fix.y == 0)
                            continue;
                    }
                }

                center += coll.fix;
                if (first)
                {
                    result.firstNormal = coll.fix;
                    result.firstTile = DefVector2(tcell*tileSize)+mapOffset+tileSize/2;
                    first = false;
                }
            }
        }
    }
    result.fix = center-oCenter;
    return result;
}

TileMapCollision TileMapCollision::FindSweepCollision(DefVector2 c1, DefVector2 c2, DefVector2 size, TilemapLayer& map, DefVector2 mapOffset, DefVector2 tileSize, std::function<DefVector2 (Vector2i,DefVector2)> callback)
{
    
    
    Vector2i minb {-1, -1};
    Vector2i maxb = map.getSize();
    
    auto inbounds = [&](const Vector2i& c)
    {
        return c.inside(minb, maxb);
    };
    
    auto isblocking = [](const int i)
    {
        return i == 1;  
    };
    
    auto iscallback = [](const int i)
    {
        return i >= 80;  
    };
    
    //if the starting position is already inside map
    TileMapCollision result = FindCollision(c1, size, map, mapOffset, tileSize, callback);
    if (result.found)
    {
        if (result.fix.x != 0 || result.fix.y != 0)
        {
            result.fix = (c1 + result.fix) - c2;
            return result;
        }
    }

    result = TileMapCollision();

    DefVector2 halfSize = size/2;


    //Vector2i mapDim = map->getDimensions();




    DefVector2 oCenter = c2;
    bool first = true;
   

    bool xnorm = false;
    bool ynorm = false;
    
    GridLine::gridIterateExtendedCellsOnLineScale<false>(tileSize, c1, c2, halfSize, [&](const std::vector<Vector2i>& vec)
    {
        DefVector2 diff = c2;
        DefVector2 cpd = c1;
        for (auto curp : vec)
        {
            DefVector2 tcell = Vector2i(curp.x,curp.y);
            

            auto t = map.getTile(curp);

            if (inbounds(curp))
            if (!(isblocking(t) || iscallback(t)))
                continue;
            
            MapTileCollision coll = MapTileCollision::FindSweepCollision(cpd,diff,size,tcell,map,tileSize);


            result.found = result.found || coll.found;
            if (coll.found)
            {

                if (inbounds(tcell))
                if (callback)
                {
                    if (iscallback(t))
                    {
                        coll.fix = callback(Vector2i(tcell),coll.fix);
                        if (coll.fix.x == 0 && coll.fix.y == 0)
                            continue;

                    }
                }
                //We put the new sweep start to the contact point of the collision
                cpd = cpd + (diff - cpd) * coll.ratio;
                //And the sweep end is at the fixed end position
                diff += coll.fix;
                if (first)
                {
                    result.firstNormal = coll.normal;
                    result.firstTile = DefVector2(tcell*tileSize)+mapOffset+tileSize/2;
                    first = false;
                }
                if (coll.normal.y != 0)
                    ynorm = true;
                if (coll.normal.x != 0)
                    xnorm = true;
            }
        }

        c1 = cpd;
        c2 = diff;


        return !(ynorm && xnorm);
    });
    if (result.found)
        result.fix = c2 - oCenter;

    return result;
}

MapTileCollision MapTileCollision::FindCollision(DefVector2 center,DefVector2 size,Vector2i tile,TilemapLayer& map,DefVector2 tileSize)
{
    MapTileCollision mpc;
    mpc.found = false;
    mpc.fix = DefVector2(0,0);

    DefVector2 tilePos = (DefVector2(tile)*tileSize)+tileSize/2;

    BoxCollision bc = BoxCollision::FindCollision(center,size,tilePos,tileSize);
    mpc.found = bc.found;
    mpc.fix = bc.fix;

    return mpc;
}

MapTileCollision MapTileCollision::FindSweepCollision(DefVector2 c1, DefVector2 c2,DefVector2 size,Vector2i tile,TilemapLayer& map,DefVector2 tileSize)
{
    MapTileCollision mpc;

    DefVector2 tilePos = (DefVector2(tile)*tileSize)+tileSize/2;

    BoxSweepCollision bsc = BoxSweepCollision::FindCollision(c1, c2, size, tilePos, tilePos, tileSize);

    mpc.found = bsc.found;
    mpc.fix = bsc.o1NewCenter - c2;
    mpc.normal = bsc.o1Normal;
    mpc.ratio = bsc.ratio;

    return mpc;
}

