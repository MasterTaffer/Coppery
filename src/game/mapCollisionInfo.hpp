#pragma once
#include "engineDefs.hpp"
#include "vector2.hpp"
class MapCollisionInfo
{
public:
    DefVector2 normal;
    DefVector2 point;
    int count;
};

class ActorCollisionInfo
{
public:
    DefVector2 normal;
    DefVector2 fix;
};
