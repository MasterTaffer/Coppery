#pragma once
#include "game/engineDefs.hpp"
#include "game/mapCollisionInfo.hpp"

class ScriptEngine;
class asIScriptObject;
class asIScriptEngine;
class asIScriptFunction;
class asITypeInfo;
class RequiredScriptType;

typedef asIScriptObject PhysicsActorType;

struct PhysicsActorData
{
    DefVector2 position;
    DefVector2 size;
    DefVector2 offset;

    unsigned cflags;
    unsigned with;
    unsigned type; 
};


//! System for communicating between AngelScript and the BroadPhase
class PhysicsActorManager
{
    int flagsOffset;
    int typeOffset;
    int withOffset;

    int positionOffset;
    int sizeOffset;
    int offsetOffset;

    asIScriptFunction* collide;
    asIScriptFunction* collideStatic;


    asITypeInfo* actorTypeInfo;
    ScriptEngine* scriptEngine;
    void rstCallback(const RequiredScriptType&);
public:
    PhysicsActorManager(ScriptEngine*);
    ~PhysicsActorManager();

    //AngelScript ? argument type calling convention. Returns nullptr on failure
    PhysicsActorType* checkIsValidPhysicsActor(void *ptr, int tid);

    void addActorRef(PhysicsActorType*);
    void releaseActor(PhysicsActorType*);

    //Assumed non-null pointers to valid data (actor must be a valid physics actor)
    // positions have offsets applied
    void getActorData(PhysicsActorType* actor, PhysicsActorData* pad);
    void setActorPosition(PhysicsActorType* actor, DefVector2 position);

    void collideActorWith(PhysicsActorType* actor, PhysicsActorType* with, ActorCollisionInfo aci);
    void collideActorWithStatic(PhysicsActorType* actor, MapCollisionInfo aci);

};