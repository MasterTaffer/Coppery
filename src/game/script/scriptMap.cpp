#include "script.hpp"
#include "vector2.hpp"
#include "color.hpp"

#include "game/tilemapLoading.hpp"
#include "game/tilemap.hpp"
#include "game/game.hpp"

#include "regHelper.hpp"

#include <angelscript.h>
#include <scripthandle/scripthandle.h>
#include <cassert>


ScriptTileCollisionCallback::ScriptTileCollisionCallback(ScriptEngine* s)
{
    se = s;
}

ScriptTileCollisionCallback::~ScriptTileCollisionCallback()
{
    if (callback)
        callback->Release();
}

void ScriptTileCollisionCallback::setCallback(asIScriptFunction* f)
{
    if (callback)
        callback->Release();
    callback = f;
}

DefVector2 ScriptTileCollisionCallback::collide(PhysicsActor* a, DefVector2 t, DefVector2 b)
{
    auto ctx = se->getContext();
    ctx->Prepare(callback);

    CScriptHandle handle;
    if (a != nullptr)
        handle.Set(a, a->GetObjectType());

    ctx->SetArgObject(0, &handle);
    ctx->SetArgObject(1, &t);
    ctx->SetArgObject(2, &b);
    ctx->Execute();
    DefVector2 f;
    f = *(DefVector2*)ctx->GetAddressOfReturnValue();
    return f;
}

/*

void ScriptEngine::getMapParameters(MapParameters* m)
{
    if (mapParamCallback == nullptr)
        return;

    mainContext->Prepare(mapParamCallback);
    mainContext->SetArgAddress(0, m);
    mainContext->Execute();
    return;
}



void ScriptEngine::scrRegisterMapSpawnObjectCallback(asIScriptFunction * cb)
{
    if (mapSpawnObjectCallback != nullptr)
        mapSpawnObjectCallback->Release();
    mapSpawnObjectCallback = cb;
}

void ScriptEngine::scrRegisterMapParamCallback(asIScriptFunction * cb)
{
    if (mapParamCallback != nullptr)
        mapParamCallback->Release();
    mapParamCallback = cb;
}


bool ScriptEngine::scrGetTileIsBlocking(DefVector2 t)
{
    return engine->getMapHandler()->getTileIsBlocking(engine->getMapHandler()->getTilePosition(t));
}
*/

void ScriptEngine::scrRegisterMapCollisionCallback(int meta,asIScriptFunction* cb)
{
    ScriptTileCollisionCallback* stcc = new ScriptTileCollisionCallback(this);
    tileCollisionCallbacks.push_back(stcc);
    stcc->setCallback(cb);

    engine->getBroadPhase()->registerTileCollisionCallback(meta, stcc);
}


TilemapLayer* factoryTilemapLayer()
{
    return new TilemapLayer();
}

Tileset* factoryTileset()
{
    return new Tileset();
}


void ScriptEngine::defineMap()
{


    int r = 0;

    r = ase->SetDefaultNamespace("Map");
    assert (r >= 0);

    /*
    r = registerGlobalFunctionAux(this,"bool LineCollision(Vector2, Vector2)", asMETHOD(GameMapHandler,getLineCollision), asCALL_THISCALL_ASGLOBAL, mapHandler);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"bool GetIsTileBlocking(Vector2)", asMETHOD(ScriptEngine,scrGetTileIsBlocking), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);

    r = ase->RegisterFuncdef("void SpawnObjectCallback(int id, int x, int y, const string &in type, const string &in name, dictionary & properties)");
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"void RegisterSpawnObjectCallback(SpawnObjectCallback@)", asMETHOD(ScriptEngine, scrRegisterMapSpawnObjectCallback), asCALL_THISCALL_ASGLOBAL, this);
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"void Load(string)", asMETHOD(Engine, loadMap), asCALL_THISCALL_ASGLOBAL, engine);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"Vector2 GetTileSize()", asMETHOD(GameMapHandler,getTileSize), asCALL_THISCALL_ASGLOBAL, mapHandler);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"Vector2 GetMapSize()", asMETHOD(GameMapHandler,getMapSize), asCALL_THISCALL_ASGLOBAL, mapHandler);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"Vector2 GetTileCenter(Vector2)", asMETHOD(GameMapHandler,getTileCenter), asCALL_THISCALL_ASGLOBAL, mapHandler);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"Tile@ GetTile(Vector2)", asMETHODPR(GameMapHandler,getTile,(DefVector2),Tile*), asCALL_THISCALL_ASGLOBAL, mapHandler);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"Tile@ GetTile(int,int)", asMETHODPR(GameMapHandler,getTile,(int,int),Tile*), asCALL_THISCALL_ASGLOBAL, mapHandler);
    assert (r >= 0);
    
    */
    
    
    r = ase->RegisterFuncdef("Vector2 TileCollisionCallback(ref @, Vector2, Vector2)");
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"void RegisterTileCollisionCallback(int,TileCollisionCallback @cb)", asMETHOD(ScriptEngine,scrRegisterMapCollisionCallback), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);
    
    
    r = ase->RegisterObjectType("Tileset",0, asOBJ_REF);
    assert (r >= 0);
    
    r = ase->RegisterObjectBehaviour("Tileset", asBEHAVE_FACTORY, "Tileset@ f()", asFUNCTION(factoryTileset), asCALL_CDECL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectBehaviour("Tileset", asBEHAVE_ADDREF, "void f()", asMETHOD(Tileset, addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("Tileset", asBEHAVE_RELEASE, "void f()", asMETHOD(Tileset, release), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Tileset", "int addDef(hash_t)", asMETHOD(Tileset, addDef), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Tileset", "int getDef(hash_t)", asMETHOD(Tileset, getDef), asCALL_THISCALL);
    assert( r >= 0 );
    
    
    r = ase->RegisterObjectType("Layer",0, asOBJ_REF);
    assert (r >= 0);
    
    r = ase->RegisterObjectBehaviour("Layer", asBEHAVE_FACTORY, "Layer@ f()", asFUNCTION(factoryTilemapLayer), asCALL_CDECL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectBehaviour("Layer", asBEHAVE_ADDREF, "void f()", asMETHOD(TilemapLayer, addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("Layer", asBEHAVE_RELEASE, "void f()", asMETHOD(TilemapLayer, release), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Layer", "void resize(int width, int height)", asMETHOD(TilemapLayer, resize), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Layer", "void clear()", asMETHOD(TilemapLayer, clear), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Layer", "void set(Vector2i pos, int value)", asMETHOD(TilemapLayer, set), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Layer", "int getTile(Vector2i pos)", asMETHOD(TilemapLayer, getTile), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Layer", "Vector2i getSize()", asMETHOD(TilemapLayer, getSize), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Layer", "void fill(Vector2i min, Vector2i to, int value)", asMETHOD(TilemapLayer, fill), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Layer", "void setTileset(Map::Tileset&)", asMETHOD(TilemapLayer, setTileset), asCALL_THISCALL);
    assert( r >= 0 );
    
        
    r = ase->RegisterObjectType("Object",0, asOBJ_REF);
    assert (r >= 0);
    
    r = ase->RegisterObjectBehaviour("Object", asBEHAVE_ADDREF, "void f()", asMETHOD(MapObject, addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("Object", asBEHAVE_RELEASE, "void f()", asMETHOD(MapObject, release), asCALL_THISCALL);
    assert( r >= 0 );
    
    
    r = ase->RegisterObjectProperty("Object","const int x", asOFFSET(MapObject,x));
    assert (r >= 0);

    r = ase->RegisterObjectProperty("Object","const int y", asOFFSET(MapObject,y));
    assert (r >= 0);
    
    r = ase->RegisterObjectProperty("Object","const int id", asOFFSET(MapObject,id));
    assert (r >= 0);
    
    r = ase->RegisterObjectProperty("Object","const string name", asOFFSET(MapObject,name));
    assert (r >= 0);

    r = ase->RegisterObjectProperty("Object","const string type", asOFFSET(MapObject,type));
    assert (r >= 0);
    
    r = ase->RegisterObjectMethod("Object", "size_t getPropertyCount()", asMETHOD(MapObject, getPropertyCount), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Object", "string getPropertyValue(size_t)", asMETHOD(MapObject, getPropertyValue), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Object", "string getPropertyName(size_t)", asMETHOD(MapObject, getPropertyName), asCALL_THISCALL);
    assert( r >= 0 );
    

    ase->SetDefaultNamespace("");
    
    
    
    
    r = ase->RegisterObjectType("Map",0, asOBJ_REF);
    assert (r >= 0);
    
    r = ase->RegisterObjectBehaviour("Map", asBEHAVE_ADDREF, "void f()", asMETHOD(Tilemap, addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("Map", asBEHAVE_RELEASE, "void f()", asMETHOD(Tilemap, release), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Map", "Map::Layer@ getLayer(hash_t)", asMETHOD(Tilemap, getLayer), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Map", "Map::Object@ getObject(size_t)", asMETHOD(Tilemap, getObject), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Map", "size_t getObjectCount()", asMETHOD(Tilemap, getObjectCount), asCALL_THISCALL);
    assert( r >= 0 );
    
    
    r = ase->SetDefaultNamespace("Map");
    assert (r >= 0);
    
    r = registerGlobalFunctionAux(this,"Map@ LoadTSMap(const string &in )", asFUNCTION(TilemapLoading::LoadTSMap), asCALL_CDECL);
    assert(r >= 0);
    
    ase->SetDefaultNamespace("");
    
    (void)(r);

}
