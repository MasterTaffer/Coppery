#pragma once
#include <vector>
#include "hash.hpp"

class TileTextures
{
public:
    Hash texture {Hash::HashUInt(0)};
    int sheetId = -1;
    float extend = 1.0f;
    bool adapt = false;
    Hash crossing {Hash::HashUInt(0)};
    Hash vertical {Hash::HashUInt(0)};
    Hash horizontal {Hash::HashUInt(0)};
};

class TileDefinition
{
public:
    TileTextures textures;
};
