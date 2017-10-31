#include "tilemap.hpp"
#include <algorithm>


void TilemapLayer::clear()
{
    for (auto& v : data)
        v = 0;
    minimum = 0;
    maximum = 0;
}


Vector2i TilemapLayer::getSize()
{
    return  {width, height};
}

void TilemapLayer::resize(int width, int height)
{
    if (width < 0 || height < 0)
    {
        return;
    }
    this->width = width;
    this->height = height;
    
    data.resize(width * height);
    clear();
}

bool TilemapLayer::validate()
{
    tilesetValidated = false;
    if (tileset)
    {
        if (minimum >= 0 && maximum <= tileset->maximumValue())
        {
            tilesetValidated = true;
            return true;
        }
    }
    return false;
}

void TilemapLayer::set(Vector2i pos, Tileset::Value value)
{
    if (pos.x < 0 || pos.x >= width)
        return;
    if (pos.y < 0 || pos.y >= height)
        return;
    
    if (value < minimum)
        minimum = value;
    if (value > maximum)
        maximum = value;
    
    data[pos.x + pos.y * width] = value;
}

void TilemapLayer::fill(Vector2i min, Vector2i to, Tileset::Value value)
{
    if (min.x < 0 || min.x >= width)
        return;
    if (min.y < 0 || min.y >= height)
        return;
    
    if (value < minimum)
        minimum = value;
    if (value > maximum)
        maximum = value;
    
    to.x = std::min(to.x, width);
    to.y = std::min(to.y, height);
    
    
    for (int y = min.y; y < to.y; y++)
    {
        auto* offset = data.data() + (y * width);
        for (int x = min.x; x < to.x; x++)
        {
            offset[x] = value;
        }
    }
    
}

void TilemapLayer::setTileset(Tileset* tileset)
{
    this->tileset = ReferenceHolder<Tileset>::create(tileset);
    validate();
}
