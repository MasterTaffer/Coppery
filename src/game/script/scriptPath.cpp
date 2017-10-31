#include "script.hpp"
#include "game/game.hpp"
#include "game/pathfinding.hpp"

#include "log.hpp"

#include <angelscript.h>
#include <cassert>

/*
class RefPathfinder : public Pathfinder
{
    int ref;
public:

    void addRef()
    {
        ref++;
    }

    void release()
    {
        ref--;
        if (ref <= 0)
        {
            delete this;
        }
    }

    RefPathfinder(GameMapHandler* ga) : Pathfinder(ga)
    {
        ref = 1;
    }

    DefVector2 getTargetPoint()
    {
        if (isDirect())
            return end;
        if (points.size() > 0)
        {
            return *points.begin();
        }
        else
            return end;
    }

    bool isAtEnd()
    {
        return (points.size()==0);
    }

    void advance()
    {
        if (points.size() > 0)
            points.pop_front();
    }
};

RefPathfinder* ScriptEngine::factoryPathfinder()
{
    return new RefPathfinder(engine->getMapHandler());
}
*/

void ScriptEngine::definePath()
{
    /*
    int r;
    r = ase->RegisterObjectType("Path",sizeof(RefPathfinder), asOBJ_REF);
    assert (r >= 0);

    ase->RegisterObjectBehaviour("Path", asBEHAVE_FACTORY, "Path@ f()", asMETHOD(ScriptEngine,factoryPathfinder), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );

    ase->RegisterObjectBehaviour("Path", asBEHAVE_ADDREF, "void f()", asMETHOD(RefPathfinder,addRef), asCALL_THISCALL);
    assert( r >= 0 );

    ase->RegisterObjectBehaviour("Path", asBEHAVE_RELEASE, "void f()", asMETHOD(RefPathfinder,release), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Path", "void setEnd(Vector2)", asMETHOD(RefPathfinder,setEnd), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Path", "void setBegin(Vector2)", asMETHOD(RefPathfinder,setBegin), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Path", "Vector2 getNext()", asMETHOD(RefPathfinder,getTargetPoint), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Path", "bool isAtEnd()", asMETHOD(RefPathfinder,isAtEnd), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Path", "bool isDirect()", asMETHOD(RefPathfinder,isDirect), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Path", "void advance()", asMETHOD(RefPathfinder,advance), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Path", "void update()", asMETHOD(RefPathfinder,update), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Path", "void setPathSize(Vector2)", asMETHOD(RefPathfinder,setPathSize), asCALL_THISCALL);
    assert( r >= 0 );
    (void)(r);
    */
}


