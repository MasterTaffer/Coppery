#pragma once
#include <unordered_map>
#include <memory>
#include <vector>
#include "game/engineDefs.hpp"

#include "quadtree.hpp"
#include "mapCollisionInfo.hpp"
#include "game/tilemap.hpp"

#include "game/physicsActorManager.hpp"


typedef DefVector2 PVector2;

/*
    System for taking care of broad phase collision detection... and the
    narrow phase as well.
*/


typedef PhysicsActorType PhysicsActor;

class GameMapHandler;
class Engine;

class CircleCollisionQuery
{
public:
    virtual void collision(PhysicsActor*,float) = 0;
};

class LineCollisionQuery
{
public:
    virtual void collision(PhysicsActor*, DefVector2) = 0;
};

class BoxCollisionQuery
{
public:
    virtual void collision(PhysicsActor*) = 0;
};


class CollisionEntity
{
public:
    DefVector2 lastPos;
    DefVector2 targetPos;
    PhysicsActor* actor;
};


class TileCollisionCallback
{
public:
    virtual DefVector2 collide(PhysicsActor*, DefVector2, DefVector2) = 0;
};

#ifdef ENGINE_INTEGER_COLLISION_DETECTION
typedef QuadTreeEntity<CollisionEntity, Vector2i> BroadPhaseQuadTreeEntity;
typedef QuadTreeHolder<CollisionEntity, Vector2i> BroadPhaseQuadTreeHolder;
#else
typedef QuadTreeEntity<CollisionEntity, DefVector2> BroadPhaseQuadTreeEntity;
typedef QuadTreeHolder<CollisionEntity, DefVector2> BroadPhaseQuadTreeHolder;
#endif



/*! \brief The collision detection system
 */
class BroadPhase
{
    DefVector2 tilemapTileSize;
    ReferenceHolder<TilemapLayer> tilemap;
    DefVector2 collisionWorldSize {0,0};
    bool active = false;
    
    std::unordered_map<PhysicsActor*, BroadPhaseQuadTreeEntity*> actors;
    Engine* engine;
    BroadPhaseQuadTreeHolder quadTree;
    void narrowPhase(CollisionEntity* a, CollisionEntity* b);
    void insertNewActorToQuadTree(std::pair<PhysicsActor* const, BroadPhaseQuadTreeEntity*> & a );

    std::unordered_map<int, TileCollisionCallback*> tileCollisionCallbacks;
    std::unique_ptr<PhysicsActorManager> pam;
    bool initialized = false;

    std::vector<PhysicsActorType*> physicsActorsToBeAdded;
    std::vector<PhysicsActorType*> physicsActorsToBeRemoved;
public:
    
    //! Register a TileCollisionCallback for the metatile of a certain type
    void registerTileCollisionCallback(int meta, TileCollisionCallback* cb);

    //! Set the MapHandler for TileMap collision detection
    void linkMapHandler(GameMapHandler* m);

    //! Add raw AngelScript object into the collision system
    void scrLinkActor(void* ptr, int tid);
    
    //! Remove AngelScript object from the collision system
    void scrRemoveActor(void* ptr, int tid);

    //! Link a PhysicsActorType into the collision system
    void linkActor(PhysicsActorType* a);
    
    //! Remove a PhysicsActorType from the collision system
    void removeActor(PhysicsActorType* a);

    //! Initialize the collision system
    void init();
    
    //! Deinitialize the collision system
    void deInit();

    //! Update the collision system and do collision detection
    void update();
    
    //! Remove all objects from the collision system
    void clear();

    //! Query a line for collisions
    void queryLineCollision(DefVector2 start, DefVector2 end,LineCollisionQuery*);
    
    //! Query a circle for collisions
    void queryCircleCollision(DefVector2 center, float radius, CircleCollisionQuery*);
    
    //! Query a box for collisions
    void queryBoxCollision(DefVector2 center, DefVector2 size, BoxCollisionQuery*);
    
    //! Set the collision tilemap and tile size
    void setTilemap(DefVector2, TilemapLayer*);
    
    //! Set collision world size
    void setCollisionWorldSize(DefVector2);
    
    //! Constructor
    BroadPhase(Engine* e);
    
    //! Destructor
    ~BroadPhase();
};

#define COLLIDE_MAP 2

#define COLLISION_IS_PROJECTILE (1 << 1)
#define COLLISION_IS_GHOST (1 << 2)
#define COLLISION_IS_STATIC (1 << 3)
#define COLLISION_CALLBACK (1 << 4)
#define COLLISION_STEP_TELEPORT (1 << 5)
