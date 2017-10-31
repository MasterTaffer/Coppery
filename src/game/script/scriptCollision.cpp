#include "script.hpp"

#include "game/broadPhase.hpp"

#include "game/collision.hpp"
#include "game/game.hpp"

#include "graphics/gfxFlags.hpp"
#include "window/inputCascade.hpp"
#include "window/window.hpp"
#include "graphics/graphics.hpp"
#include "graphics/particle.hpp"
#include "graphics/graphicsData.hpp"

#include "sound/sound.hpp"

#include "log.hpp"
#include "regHelper.hpp"

#include <scripthandle/scripthandle.h>
#include <angelscript.h>
#include <cassert>
#include <cmath>


void ScriptEngine::scrRunCollisionDetection()
{
    
    CircularCallLockHolder lock(lock_runCollisionDetection);
    if (lock.lockDenied())
        return;

    mainContext->PushState();

    engine->getBroadPhase()->update();

    mainContext->PopState();
}

void ScriptEngine::scrClearCollisionDetection()
{

    
    CircularCallLockHolder lock(lock_runCollisionDetection);
    if (lock.lockDenied())
        return;

    engine->getBroadPhase()->clear();

}




class ScriptCircleCollisionQuery : public CircleCollisionQuery
{
    asIScriptContext* ctx;
    asIScriptFunction * func;
public:
    ScriptCircleCollisionQuery(ScriptEngine* se, asIScriptContext* ctx, asIScriptFunction * func)
    : ctx(ctx), func(func)
    {
    }

    void collision(PhysicsActor* c, float r)
    {
        asIScriptFunction* cb = func;
        if (func->GetFuncType() == asFUNC_DELEGATE)
            cb = func->GetDelegateFunction();

        ctx->Prepare(cb);

        if (func->GetFuncType() == asFUNC_DELEGATE)
            ctx->SetObject(func->GetDelegateObject());

        CScriptHandle handle;
        handle.Set(c, c->GetObjectType());
        ctx->SetArgObject(0,&handle);
        ctx->SetArgFloat(1,r);

        ctx->Execute();
    }

};

void ScriptEngine::scrCircleCollisionQuery(DefVector2 pos,float r,asIScriptFunction *cb)
{
    auto* ctx = ase->RequestContext();

    ScriptCircleCollisionQuery sccq(this,ctx,cb);
    engine->getBroadPhase()->queryCircleCollision(pos,r,&sccq);

    cb->Release();

    ase->ReturnContext(ctx);
}


class ScriptLineCollisionQuery : public LineCollisionQuery
{
    asIScriptContext* ctx;
    asIScriptFunction * func;
public:
    ScriptLineCollisionQuery(ScriptEngine* se, asIScriptContext* c, asIScriptFunction * cb)
    {
        ctx = c;
        func = cb;
    }

    void collision(PhysicsActor* c, DefVector2 p1 ) override
    {
        asIScriptFunction* cb = func;
        if (func->GetFuncType() == asFUNC_DELEGATE)
            cb = func->GetDelegateFunction();

        ctx->Prepare(cb);

        if (func->GetFuncType() == asFUNC_DELEGATE)
            ctx->SetObject(func->GetDelegateObject());


        CScriptHandle handle;
        if (c != nullptr)
            handle.Set(c, c->GetObjectType());

        ctx->SetArgObject(0, &handle);
        ctx->SetArgObject(1, &p1);

        ctx->Execute();
    }

};

void ScriptEngine::scrLineCollisionQuery(DefVector2 p1, DefVector2 p2, asIScriptFunction *cb)
{
    auto* ctx = ase->RequestContext();

    ScriptLineCollisionQuery slcq(this, ctx, cb);
    engine->getBroadPhase()->queryLineCollision(p1,p2, &slcq);

    cb->Release();

    ase->ReturnContext(ctx);
}

class ScriptBoxCollisionQuery : public BoxCollisionQuery
{
    asIScriptContext* ctx;
    asIScriptFunction * func;
public:
    ScriptBoxCollisionQuery(ScriptEngine* se, asIScriptContext* c, asIScriptFunction * cb)
    {
        ctx = c;
        func = cb;
    }

    void collision(PhysicsActor* c) override
    {
        asIScriptFunction* cb = func;
        if (func->GetFuncType() == asFUNC_DELEGATE)
            cb = func->GetDelegateFunction();

        ctx->Prepare(cb);

        if (func->GetFuncType() == asFUNC_DELEGATE)
            ctx->SetObject(func->GetDelegateObject());


        CScriptHandle handle;
        if (c != nullptr)
            handle.Set(c, c->GetObjectType());

        ctx->SetArgObject(0, &handle);

        ctx->Execute();
    }

};

void ScriptEngine::scrBoxCollisionQuery(DefVector2 cent, DefVector2 size, asIScriptFunction *cb)
{
    auto* ctx = ase->RequestContext();

    ScriptBoxCollisionQuery slcq(this, ctx, cb);
    engine->getBroadPhase()->queryBoxCollision(cent, size, &slcq);
    

    cb->Release();

    ase->ReturnContext(ctx);
}



bool FindTilemapLineCollision(DefVector2 p1, DefVector2 p2, TilemapLayer& map, DefVector2 tileSize,  DefVector2& pos, DefVector2& norm)
{
    auto t = TileMapLineCollision::FindCollision(p1, p2, map, {0,0}, tileSize);
    
    if (t.found == true)
    {
        pos = t.position;
        norm = t.normal;
        return true;
    }
    
    return false;
}


void ScriptEngine::defineCollisionFunctions()
{
    int r = 0;



    BroadPhase* broadPhase = nullptr;

    if (!compilerOnly)
    {
        broadPhase = engine->getBroadPhase();
    }



    r = ase->SetDefaultNamespace("Collision");
    assert(r >= 0);
    r = registerGlobalFunctionAux(this,"void RunDetection()", asMETHOD(ScriptEngine, scrRunCollisionDetection), asCALL_THISCALL_ASGLOBAL, this);
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"void RemoveAll()", asMETHOD(ScriptEngine, scrClearCollisionDetection), asCALL_THISCALL_ASGLOBAL, this);
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"void Add(?&in)", asMETHOD(BroadPhase, scrLinkActor), asCALL_THISCALL_ASGLOBAL, broadPhase);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"void Remove(?&in)", asMETHOD(BroadPhase, scrRemoveActor), asCALL_THISCALL_ASGLOBAL, broadPhase);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"void SetMap(Vector2 tileSize, Map::Layer& map)", asMETHOD(BroadPhase, setTilemap), asCALL_THISCALL_ASGLOBAL, broadPhase);
    assert (r >= 0);
    
    r = registerGlobalFunctionAux(this,"void SetWorldSize(Vector2)", asMETHOD(BroadPhase, setCollisionWorldSize), asCALL_THISCALL_ASGLOBAL, broadPhase);
    assert (r >= 0);

    r = ase->RegisterFuncdef("void QueryCircleCallback(ref @, float)");
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"void Query(Vector2, float, QueryCircleCallback@)", asMETHOD(ScriptEngine, scrCircleCollisionQuery), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);

    r = ase->RegisterFuncdef("void QueryLineCallback(ref @, Vector2)");
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"void Query(Vector2, Vector2, QueryLineCallback@)", asMETHOD(ScriptEngine, scrLineCollisionQuery), asCALL_THISCALL_ASGLOBAL, this);
    assert(r >= 0);

    r = ase->RegisterFuncdef("void QueryBoxCallback(ref @)");
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"void Query(Vector2, Vector2, QueryBoxCallback@)", asMETHOD(ScriptEngine, scrBoxCollisionQuery), asCALL_THISCALL_ASGLOBAL, this);
    assert(r >= 0);


    r = ase->RegisterObjectType("MapCollisionInfo",0, asOBJ_REF | asOBJ_NOCOUNT);
    assert (r >= 0);

    r = ase->RegisterObjectProperty("MapCollisionInfo","Vector2 normal",asOFFSET(MapCollisionInfo,normal));
    assert (r >= 0);

    r = ase->RegisterObjectProperty("MapCollisionInfo","Vector2 point",asOFFSET(MapCollisionInfo,point));
    assert (r >= 0);

    r = ase->RegisterObjectType("ActorCollisionInfo",0, asOBJ_REF | asOBJ_NOCOUNT);
    assert (r >= 0);

    r = ase->RegisterObjectProperty("ActorCollisionInfo","Vector2 normal",asOFFSET(ActorCollisionInfo,normal));
    assert (r >= 0);

    r = ase->RegisterObjectProperty("ActorCollisionInfo","Vector2 fix",asOFFSET(ActorCollisionInfo,fix));
    assert (r >= 0);
    
    

    r = ase->SetDefaultNamespace("Collision::Extra");
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"bool MapLine(Vector2, Vector2, Map::Layer&, Vector2, Vector2 &out pos, Vector2 &out norm)", asFUNCTION(FindTilemapLineCollision), asCALL_CDECL);
    assert (r >= 0);
    
    
    r = ase->SetDefaultNamespace("");
    assert(r >= 0);




    RequiredScriptType rst;
    rst.name = "PhysicsActor";

    RequiredScriptType::Function fun;

    fun.name = "collide";
    fun.declaration = "void collide(PhysicsActor@, const Collision::ActorCollisionInfo&)";

    rst.functions.push_back(fun);

    fun.name = "collideStatic";
    fun.declaration = "void collideStatic(const Collision::MapCollisionInfo&)";

    rst.functions.push_back(fun);

    RequiredScriptType::Property p;

    p.type = (int)asTYPEID_UINT32; p.name = "id";
    rst.properties.push_back(p);

    p.type = (int)asTYPEID_UINT32; p.name = "collisionFlags";
    rst.properties.push_back(p);

    p.type = (int)asTYPEID_UINT32; p.name = "collisionType";
    rst.properties.push_back(p);

    p.type = (int)asTYPEID_UINT32; p.name = "collisionWith";
    rst.properties.push_back(p);

    int v2id = ase->GetTypeIdByDecl("Vector2");

    p.type = v2id; p.name = "position";
    rst.properties.push_back(p);

    p.type = v2id; p.name = "size";
    rst.properties.push_back(p);

    p.type = v2id; p.name = "collisionOffset";
    rst.properties.push_back(p);

    addRequiredScriptType(rst);

    (void)(r);
}
