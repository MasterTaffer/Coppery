#include "game/physicsActorManager.hpp"
#include "game/script/script.hpp"
#include <angelscript.h>
#include "log.hpp"

PhysicsActorManager::PhysicsActorManager(ScriptEngine* se) :
    actorTypeInfo(nullptr),
    collide(nullptr),
    collideStatic(nullptr)
{
    if (se == nullptr)
        throw std::runtime_error("nullptr passed to PhysicsActorManager constructor");
    this->scriptEngine = se;

    if (!se->successfulInit())
    {
        Log << "PhysicsActorManager constructed when ScriptEngine was not initialized" << Trace(CHash("Warning"));
        return;
    }



    const RequiredScriptType* rst = se->getRequiredScriptType("PhysicsActor");
    if (rst && rst->found)
    {
        rstCallback(*rst);
    }
    else
    {
        Log << "PhysicsActor not defined: collision detection disabled" << Trace(CHash("Warning"));
    }
}

PhysicsActorManager::~PhysicsActorManager()
{
    if (actorTypeInfo)
        actorTypeInfo->Release();
    actorTypeInfo = nullptr;

    if (collide)
        collide->Release();
    collide = nullptr;

    if (collideStatic)
        collideStatic->Release();
    collideStatic = nullptr;
}

void PhysicsActorManager::rstCallback(const RequiredScriptType& rst)
{
    actorTypeInfo = rst.typeInfo;
    actorTypeInfo->AddRef();
    
    flagsOffset = rst.getProperty("collisionFlags")->offset;
    typeOffset = rst.getProperty("collisionType")->offset;
    withOffset = rst.getProperty("collisionWith")->offset;
    positionOffset = rst.getProperty("position")->offset;
    sizeOffset = rst.getProperty("size")->offset;
    offsetOffset = rst.getProperty("collisionOffset")->offset;

    collide = rst.getFunction("collide")->ptr;
    collideStatic = rst.getFunction("collideStatic")->ptr;

    collide->AddRef();
    collideStatic->AddRef();
}

PhysicsActorType* PhysicsActorManager::checkIsValidPhysicsActor(void *ptr, int tid)
{
    return scriptEngine->validateObject(ptr, tid, actorTypeInfo);
}

#define ActorOffset(x, offset, type) (*((type*)(((char*)x) + offset)))

void PhysicsActorManager::getActorData(PhysicsActorType* actor, PhysicsActorData* pad)
{
    pad->cflags = ActorOffset(actor, flagsOffset, unsigned int);
    pad->with = ActorOffset(actor, withOffset, unsigned int);
    pad->type = ActorOffset(actor, typeOffset, unsigned int);

    pad->offset = ActorOffset(actor, offsetOffset, DefVector2);
    pad->size = ActorOffset(actor, sizeOffset, DefVector2);
    

    pad->position = ActorOffset(actor, positionOffset, DefVector2);
    pad->position += pad->offset;
}


void PhysicsActorManager::collideActorWith(PhysicsActorType* actor, PhysicsActorType* with, ActorCollisionInfo aci)
{
    asIScriptContext* ctx = scriptEngine->getContext();

    ctx->Prepare(collide);
    ctx->SetArgObject(0,with);
    ctx->SetArgObject(1,&aci);
    ctx->SetObject(actor);
    ctx->Execute();
}


void PhysicsActorManager::collideActorWithStatic(PhysicsActorType* actor, MapCollisionInfo mci)
{
    asIScriptContext* ctx = scriptEngine->getContext();

    ctx->Prepare(collideStatic);
    ctx->SetArgObject(0,&mci);
    ctx->SetObject(actor);
    ctx->Execute();

}

void PhysicsActorManager::setActorPosition(PhysicsActorType* actor, DefVector2 pos)
{
    DefVector2 offs = ActorOffset(actor, offsetOffset, DefVector2);
    ActorOffset(actor, positionOffset, DefVector2) = pos - offs;
}

void PhysicsActorManager::addActorRef(PhysicsActorType* actor)
{
    actor->AddRef();
}

void PhysicsActorManager::releaseActor(PhysicsActorType* actor)
{
    actor->Release();
}
