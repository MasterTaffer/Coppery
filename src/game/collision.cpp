#include <algorithm>
#include "collision.hpp"

LineIntersection LineIntersection::FindCollision(DefVector2 p1, DefVector2 p2, DefVector2 p3, DefVector2 p4)
{
    //Now THIS is an algorithm!

    LineIntersection li;
    li.found = false;
    DefVector2 l1 = p2-p1;
    DefVector2 l2 = p4-p3;
    double s, t;
    //Something something vector projection, can't remember, copy pasted 6 years old code
    s = (-l1.y*(p1.x-p3.x)+l1.x*(p1.y-p3.y))/(-l2.x*l1.y+l1.x*l2.y);
    t = ( l2.x*(p1.y-p3.y)-l2.y*(p1.x-p3.x))/(-l2.x*l1.y+l1.x*l2.y);
    if ( s>= 0 && s <= 1 && t >= 0 && t <= 1)
    {

        li.pos.x= p1.x+t*l1.x;
        li.pos.y=p1.y+t*l1.y;
        li.found = true;
    }
    return li;

}


LineIntersection LineIntersection::FindCollisionRatio(DefVector2 p1, DefVector2 p2, DefVector2 p3, DefVector2 p4)
{
    LineIntersection li;
    li.found = false;
    DefVector2 l1 = p2-p1;
    DefVector2 l2 = p4-p3;
    double s, t;
    s = (-l1.y*(p1.x-p3.x)+l1.x*(p1.y-p3.y))/(-l2.x*l1.y+l1.x*l2.y);
    t = ( l2.x*(p1.y-p3.y)-l2.y*(p1.x-p3.x))/(-l2.x*l1.y+l1.x*l2.y);
    if ( s>= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        li.ratio1 = t;
        li.ratio2 = s;
        li.found = true;
    }
    return li;
}


BoxCollision BoxCollision::FindCollision(DefVector2 center1, DefVector2 size1, DefVector2 center2, DefVector2 size2)
{
    BoxCollision result;
    size1 = size1/2;
    size2 = size2/2;

    double ydir = 1;
    double xdir = 1;
    DefVector2 delta = center2-center1;
    if (delta.y < 0)
        ydir = -1;
    if (delta.x < 0)
        xdir = -1;
    delta.x *= xdir;
    delta.y *= ydir;
    DefVector2 combinedSize = size1+size2;
    DefVector2 coll = delta-combinedSize;

    if (coll.x < 0 && coll.y < 0)
    {
        DefVector2 retvec;
        if (coll.x > coll.y)
            retvec = {coll.x,0};
        else
            retvec = {0,coll.y};
        retvec.x *= xdir;
        retvec.y *= ydir;
        result.fix = retvec;
        result.found = true;
        return result;
    }
    else
        return result;

}

BoxLineCollision BoxLineCollision::FindCollision(DefVector2 center, DefVector2 size, DefVector2 p1, DefVector2 p2)
{
    BoxLineCollision result;
    size/=2;
    DefVector2 t = center-size;
    DefVector2 b = center+size;

    bool side;

    //If all 4 points of the box is on the same side of the line, no collision is found
    #define get_line_side(xx,yy) ((p2.y-p1.y)*xx+(p1.x-p2.x)*yy+(p2.x*p1.y-p1.x*p2.y))

    side = (get_line_side(t.x,t.y)>0);
    if (side != (get_line_side(t.x,b.y)>0))
        goto end;
    if (side != (get_line_side(b.x,b.y)>0))
        goto end;
    if (side != (get_line_side(b.x,t.y)>0))
        goto end;
    return result;
    end:

    //Otherwise, the line either goes through the box
    if (p1.x > b.x && p2.x > b.x)
        return result;

    if (p1.x < t.x && p2.x < t.x)
        return result;

    if (p1.y > b.y && p2.y > b.y)
        return result;

    if (p1.y < t.y && p2.y < t.y)
        return result;

    result.found = true;
    return result;
}


BoxLineCollision BoxLineCollision::FindCollisionRatio(DefVector2 center, DefVector2 size, DefVector2 p1, DefVector2 p2)
{
    BoxLineCollision result;
    size/=2;
    const DefVector2 t = center-size; //topLeft
    const DefVector2 b = center+size; //bottomRight
    const DefVector2 bottomLeft = {t.x, b.y};
    const DefVector2 topRight = {b.x, t.y};

    //Cohen-Sutherland: https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
    int p1mask, p2mask;

    constexpr int left = 1;
    constexpr int right = 2;
    constexpr int top = 4;
    constexpr int bottom = 8;

    auto compmask = [&](const DefVector2& v) -> int
    {
        int mask = 0;
        if (v.x <= t.x)
            mask |= left;
        else if (v.x >= b.x)
            mask |= right;
        if (v.y <= t.y)
            mask |= top;
        else if (v.y >= b.y)
            mask |= bottom;
        return mask;
    };

    p1mask = compmask(p1);
    p2mask = compmask(p2);
    if (p1mask & p2mask) //p1mask & p2mask
    {
        return result;
    }

    if (p1mask == p2mask) //p1mask == 0 == p2mask
    {
        result.inside = true;
        return result;
    } 

    LineIntersection li;

    auto testc = [&](int dir, const DefVector2& lp1, const DefVector2& lp2, const DefVector2& normal) -> bool
    {
        //If collision occurs, the entry side must be set
        if ((p1mask & dir) == 0)
            return false;
        li = LineIntersection::FindCollisionRatio(p1, p2, lp1, lp2);
        if (li.found)
        {
            result.found = true;
            result.ratio = li.ratio1;
            result.normal = normal;
            return true;
        }
        return false;
    };



    if (testc(left, t, bottomLeft, {-1,0}))
        return result;
    if (testc(top, t, topRight, {0,-1}))
        return result;
    if (testc(bottom, bottomLeft, b, {0, 1}))
        return result;
    if (testc(right, topRight, b, {1, 0}))
        return result;
  
    return result;
}

BoxLineCollision BoxLineCollision::FindCollisionPoint(DefVector2 center, DefVector2 size, DefVector2 p1, DefVector2 p2)
{
    BoxLineCollision result = FindCollisionRatio(center, size, p1, p2);
    if (result.found)
    {
        result.point = p1 + (p2 - p1) * result.ratio;
    }
  
    return result;
}

BoxSweepCollision BoxSweepCollision::FindCollision(DefVector2 o1center1, DefVector2 o1center2, DefVector2 o1size, DefVector2 o2center1, DefVector2 o2center2, DefVector2 o2size, bool o1Static, bool o2Static)
{
    DefVector2 o1vel = o1center2 - o1center1;
    DefVector2 o2vel = o2center2 - o2center1;

    //take a relativistic approach and assume only the first object is moving
    DefVector2 combinedVel = o1vel - o2vel;


    //now do some fun minkowski sum stuff, you know
    DefVector2 combinedSize = o1size + o2size;


    BoxLineCollision blc = BoxLineCollision::FindCollisionRatio(o2center1, combinedSize, o1center1, o1center1+combinedVel);
    BoxSweepCollision bsc;
    bsc.found = false;

    if (blc.inside)
    {
        BoxCollision bc = BoxCollision::FindCollision(o1center2, o1size, o2center2, o2size);
        if (bc.found == true) //TODO, how the heck this can be false?
        {
            bsc.found = true;
            bsc.ratio = 0;
            if (o2Static)
            {
                bsc.o1NewCenter = o1center2 + bc.fix;
            }
            else if (o1Static)
            {
                bsc.o2NewCenter = o2center2 - bc.fix;
            }
            else if (!(o1Static || o2Static))
            {
                bsc.o1NewCenter = o1center2 + bc.fix / 2;
                bsc.o2NewCenter = o2center2 - bc.fix / 2;
            }

            bsc.o1Normal = bc.fix.normalize();
            return bsc;
        }
    }
    
    if (blc.found)
    {
        bsc.found = true;
        bsc.ratio = blc.ratio;

        DefVector2 anormal = DefVector2(std::abs(blc.normal.x), std::abs(blc.normal.y));

        DefVector2 ratio;
        ratio.x = anormal.x * blc.ratio + (1 - anormal.x);

        ratio.y = anormal.y * blc.ratio + (1 - anormal.y);



        bsc.o1NewCenter = o1center1 + o1vel * ratio;
        bsc.o2NewCenter = o2center1 + o2vel * ratio;

        if (blc.normal.x == 0)
        {
            bsc.o1NewCenter.x = o1center2.x;
            bsc.o2NewCenter.x = o2center2.x;
        }

        if (blc.normal.y == 0)
        {
            bsc.o1NewCenter.y = o1center2.y;
            bsc.o2NewCenter.y = o2center2.y;
        }

        bsc.o1Normal = blc.normal;
    }
    return bsc;
}

//The same as above except this does finer checking 
BoxSweepCollision BoxSweepCollision::FindCollisionSlide(DefVector2 o1center1, DefVector2 o1center2, DefVector2 o1size, DefVector2 o2center1, DefVector2 o2center2, DefVector2 o2size, bool o1Static, bool o2Static)
{
    //First check the collision with the above algorithm
    auto bsc = FindCollision(o1center1, o1center2, o1size, o2center1, o2center2, o2size, o1Static, o2Static);

    if (bsc.found == true)
    {
        //The following algorithm prevents "sticking"
        //if object A and B are moving to the same direction
        //the regular algorithm might make them stuck together
        //even if they should be moving apart

        DefVector2 o1vel = o1center2 - o1center1;
        DefVector2 o2vel = o2center2 - o2center1;

        DefVector2::dimensionType o1cx, o2cx;
        DefVector2::dimensionType o1dx, o2dx;

        DefVector2 combinedSizeDiff = (o1size + o2size) / 2;
        DefVector2 endDimMask = DefVector2(0, 0);
        DefVector2 endDimMaskOpposite = DefVector2(0, 0);

        if (bsc.o1Normal.x == 0)
        {
            o1dx = o1vel.y;
            o2dx = o2vel.y;
            o1cx = o1center1.y;
            o2cx = o2center1.y;

            combinedSizeDiff.x = 0;
            endDimMask.x = 1;

            endDimMaskOpposite.y = 1;
        }
        else
        {
            o1dx = o1vel.x;
            o2dx = o2vel.x;
            o1cx = o1center1.x;
            o2cx = o2center1.x;

            combinedSizeDiff.y = 0;
            endDimMask.y = 1;

            endDimMaskOpposite.x = 1;
        }

        //if o1dx & o2dx are the same sign
        if ((o1dx * o2dx) > 0)
        {
            //If the objects move to opposing directions
            //along the collision axis

            //Multiply positions with the sign
            o1cx *= o1dx;
            o2cx *= o1dx;

            combinedSizeDiff.x = std::copysign(combinedSizeDiff.x, -o1dx);
            combinedSizeDiff.y = std::copysign(combinedSizeDiff.y, -o1dx);

            if (o1cx > o2cx)
            {
                bsc.o1NewCenter = o1center2;
                bsc.o2NewCenter = (o1center2 + combinedSizeDiff) * endDimMaskOpposite + (o2center2) * endDimMask;
            }
            else if (o1cx < o2cx)
            {
                bsc.o1NewCenter = (o2center2 + combinedSizeDiff) * endDimMaskOpposite + (o1center2) * endDimMask;
                bsc.o2NewCenter = o2center2;
            }
        }

    }
    return bsc;
}
