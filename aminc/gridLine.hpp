#pragma once
#include <iostream>
#include "vector2.hpp"

#include <cassert>
#if __cplusplus >= 201103L
    #include <type_traits>
#endif


//! \brief 2d-grid cell iteration tools
namespace GridLine
{




/*! \brief Iterates through cells on a line in a 2d-grid
 * 
 * The function goes through the cells of a 2d-grid which happen to align on
 * the line defined by the starting point \p a and end point \p b. The
 * function calls the \p func for each cell encountered with the cell's
 * coordinates. If \p func returns false, the iteration is interrupted
 * and the function returns.
 * 
 * The function has the concept of integer cell coordinates which is passed
 * to \p func. One can convert the fractional real coordinates to cell 
 * coordinates with function `(x, y) => (Math.floor(x), Math.floor(y))`.
 * 
 * \tparam checkFirst whether the function should check the starting cell
 * \tparam floatingPoint the floating point type used in calculations
 * \param a the starting point for the line
 * \param b the end point for the line
 * \param func the callback function
 * \return The point where the iteration finished.
 */
template <bool checkFirst, typename floatingPoint>
Vector2<floatingPoint> gridIterateCellsOnLine(Vector2<floatingPoint> a, Vector2<floatingPoint> b, std::function<bool(const Vector2i& cellcoord)> func)
{
    #if __cplusplus >= 201103L
        static_assert(std::is_floating_point<floatingPoint>::value, "Template specialization parameter must be a floating point type.");
    #endif

	Vector2i sPos, ePos;

	//sPos tulisi olla alaspain pyöristetty solukoordinaatti
	sPos = a;

	//jos meillä on negatiivisia koordinaatteja, pyöristetään alaspäin, ei kohti nollaa
	if (a.x < 0)
        sPos.x -= 1;

    if (a.y < 0)
        sPos.y -= 1;
    
    ePos = b;
    if (b.x < 0)
        ePos.x -= 1;
    if (b.y < 0)
        ePos.y -= 1;

    //Hoidetaan ensimmäinen solu alta pois
    if (checkFirst)
        if (!func(sPos))
            return a;

    //jos alku ja loppusolut ovat sama
	if (sPos == ePos)
    {
        return a;
    }

    //Erikoistapaus, kun suora on täysin pystysuora

    if (a.x == b.x)
    {
        int dif = ePos.y-sPos.y;
        Vector2i dir;

        if (dif < 0)
        {
            dir = Vector2i(0, -1);
            dif = -dif;
        }
        else
        {
            dir = Vector2i(0, 1);
        }

        while (sPos != ePos)
        {
            sPos+=dir;
            if (!func(sPos))
            {
                return {a.x, double(sPos.y + (dir.y > 0 ? 0 : 1))};
            }
        }
        return b;
    }


    floatingPoint yDelta = (a.y-sPos.y);
    floatingPoint xDelta = (a.x-sPos.x);

    assert(!(xDelta < 0 || yDelta < 0));

    floatingPoint yDif = b.y-a.y;
	floatingPoint xDif = b.x-a.x;

    Vector2i xdir(1,0);
    Vector2i ydir(0,1);

    if (xDif < 0)
    {
        xDif = -xDif;
        xdir = Vector2i(-1, 0);
    }
    else
        xDelta = 1-xDelta;

    if (yDif < 0)
    {
        yDif = -yDif;
        ydir = Vector2i(0, -1);

    }
    else
        yDelta = 1-yDelta;


    floatingPoint curv = yDif/xDif;
    floatingPoint icurv = xDif/yDif;

    //Kuinka monen solun päässä pisteet ovat

    //Kyseessä on manhattan-etäisyys, koska algoritmi siirtyy solusta toiseen
    //vain vain kardinaalisissa suunnissa

    //kertoo myös kuinka monta kertaa seuraavassa while loopissa ollaan

    unsigned int ark = std::abs(sPos.x-ePos.x)+std::abs(sPos.y-ePos.y);

    //Iterointi tulisi lopettaa, kun saavutaan linjan päätyyn (sPos == ePos)
    //Mutta floating point matikka ei takaa, että viiva meneekään ihan suoraan

    //Joissain tapauksissa, kun on kyseessä hyvin suuret luvut, niin algoritmi ei välttämättä mene ihan nappiin
    //ja viiva meneekin "huti" loppupisteestä

    //Käytetään siksi vähän erilaisempaa logiikkaa

    //Jos (ark == 0) niin (sPos == ePos), ja lopetetaan iterointi
    //samalla varmistetaan, että algoritmi päättyy ainakin joskus
    while (ark > 0)
    {
        floatingPoint yAdd = xDelta*curv;

        if (yAdd > yDelta)
        {
            xDelta -= yDelta*icurv;
            yDelta = 1.0;

            sPos += ydir;
        }
        else
        {
            yDelta -= yAdd;
            xDelta = 1.0;

            sPos += xdir;
        }
        if (!func(sPos))
        {
            Vector2<floatingPoint> ret = sPos;
            ret.x += xdir.x > 0 ? 1 - xDelta : xDelta;
            ret.y += ydir.y > 0 ? 1 - yDelta : yDelta;
    		return ret;
        }
        ark--;
    }
    return b;
}



/*! \brief Iterates through cells on a line in a 2d-grid with line width
 * 
 * Similar to gridIterateCellsOnLine(Vector2<floatingPoint> a, Vector2<floatingPoint> b, std::function<bool(const Vector2i& cellcoord)> func)
 * except with parameter \p cellExtraSize. The cellExtraSize essentially
 * defines the width of the line going through the grid.
 */
template <bool ignoreInitial, typename floatingPoint>
void gridIterateExtendedCellsOnLine(Vector2<floatingPoint> a, Vector2<floatingPoint> b, Vector2<floatingPoint> cellExtraSize, std::function<bool(const std::vector<Vector2i>& a)> func)
{
    #if __cplusplus >= 201103L
        static_assert(std::is_floating_point<floatingPoint>::value, "Template specialization parameter must be a floating point type.");
    #endif


    //Very similar to the algorithm above...

    typedef Vector2<floatingPoint> vec2;
    Vector2i sPos, ePos;

    

    auto genFirstCell = [](const vec2& v) -> Vector2i
    {
        return {int(v.x < 0 ? v.x - 1 : v.x), int(v.y < 0 ? v.y - 1 : v.y)};
    };

    sPos = genFirstCell(a);
    ePos = genFirstCell(b);


    Vector2i minSPos = genFirstCell(a - cellExtraSize);
    Vector2i maxSPos = genFirstCell(a + cellExtraSize);

    std::vector<Vector2i> cells;
    cells.reserve((int)((cellExtraSize.x + 1)*(cellExtraSize.y + 1)));

    if (!ignoreInitial)
    {

        //except that the start position is in fact a rectangle
        for (int i = minSPos.x; i <= maxSPos.x; i++)
        for (int j = minSPos.y; j <= maxSPos.y; j++)
        {
            cells.push_back({i,j});
        }

        if (!func(cells))
            return; 
        cells.clear();
    }


    floatingPoint yDif = b.y-a.y;
    floatingPoint xDif = b.x-a.x;

    vec2 direction = vec2(copysign(1.0, xDif), copysign(1.0, yDif));
    Vector2i idir = Vector2i(direction.x > 0 ? 1 : -1, direction.y > 0 ? 1 : -1);

    a += direction*cellExtraSize;
    b += direction*cellExtraSize;

    sPos = genFirstCell(a);
    ePos = genFirstCell(b);
    if (sPos == ePos)
    {
        return;
    }
    if (sPos.x == ePos.x)
    {
        int dif = ePos.y-sPos.y;
        Vector2i dir;

        if (dif < 0)
        {
            dir = Vector2i(0, -1);
            dif = -dif;
        }
        else
        {
            dir = Vector2i(0, 1);
        }

        while (sPos != ePos)
        {
            sPos+=dir;
            for (int i = minSPos.x; i <= maxSPos.x; i++)
                cells.push_back({i,sPos.y});

            if (!func(cells))
                return;
            cells.clear();
        }
        return;
    }


    floatingPoint yDelta = (a.y-sPos.y);
    floatingPoint xDelta = (a.x-sPos.x);


    assert(!(xDelta < 0 || yDelta < 0));


    Vector2i xdir(1,0);
    Vector2i ydir(0,1);

    if (xDif < 0)
    {
        xDif = -xDif;
        xdir = Vector2i(-1, 0);
    }
    else
        xDelta = 1-xDelta;

    if (yDif < 0)
    {
        yDif = -yDif;
        ydir = Vector2i(0, -1);

    }
    else
        yDelta = 1-yDelta;


    floatingPoint curv = yDif/xDif;
    floatingPoint icurv = xDif/yDif;


    unsigned int ark = std::abs(sPos.x-ePos.x)+std::abs(sPos.y-ePos.y);
    while (ark > 0)
    {
        floatingPoint yAdd = xDelta*curv;

        if (yAdd > yDelta)
        {
            xDelta -= yDelta*icurv;
            yDelta = 1.0;


            sPos += ydir;
            floatingPoint dd = xDelta + cellExtraSize.x * 2;
            int hiy = (int)dd;

            for (int i = hiy; i >= 0; i--)
            {
                cells.push_back({sPos.x - i * idir.x, sPos.y});
            }
            if (!func(cells))
                return;
            cells.clear();
        }
        else
        {
            yDelta -= yAdd;
            xDelta = 1.0;

            sPos += xdir;
            floatingPoint dd = yDelta + cellExtraSize.y * 2;
            int hix = (int)dd;
            for (int i = hix; i >= 0; i--)
            {
                cells.push_back({sPos.x, sPos.y - i * idir.y});
            }
            if (!func(cells))
                return;
            cells.clear();
        }

        ark--;
    }
}

/*! \brief Iterates through cells on a line in a 2d-grid with scaling
 * 
 * Similar to gridIterateCellsOnLine(Vector2<floatingPoint> a, Vector2<floatingPoint> b, std::function<bool(const Vector2i& cellcoord)> func)
 * but with a parameter \p cellSize to signify the size of a single grid cell.
 * 
 */
template <bool ignoreFirst = false, typename floatingPoint>
Vector2<floatingPoint> gridIterateCellsOnLineScale(Vector2<floatingPoint> cellSize, Vector2<floatingPoint> a, Vector2<floatingPoint> b, std::function<bool(const Vector2i& a)> func)
{
    return gridIterateCellsOnLine<ignoreFirst>(a/cellSize, b/cellSize, func) * cellSize;
}

/*! \brief Iterates through cells on a line in a 2d-grid with extra parameters
 * 
 * Similar to gridIterateCellsOnLine(Vector2<floatingPoint> a, Vector2<floatingPoint> b, std::function<bool(const Vector2i& cellcoord)> func)
 * but with parameters \p cellSize and \p cellExtraSize, like in functions
 * gridIterateCellsOnLineScale(Vector2<floatingPoint> cellSize, Vector2<floatingPoint> a, Vector2<floatingPoint> b, std::function<bool(const Vector2i& a)> func)
 * and
 * gridIterateExtendedCellsOnLine(Vector2<floatingPoint> a, Vector2<floatingPoint> b, Vector2<floatingPoint> cellExtraSize, std::function<bool(const std::vector<Vector2i>& a)> func)
 * respectively.
 * 
 */
template <bool ignoreInitial, typename floatingPoint>
void gridIterateExtendedCellsOnLineScale(Vector2<floatingPoint> cellSize, Vector2<floatingPoint> a, Vector2<floatingPoint> b, Vector2<floatingPoint> cellExtraSize, std::function<bool(const std::vector<Vector2i>& a)> func)
{
    gridIterateExtendedCellsOnLine<ignoreInitial>(a/cellSize, b/cellSize, cellExtraSize/cellSize, func);
}




}
