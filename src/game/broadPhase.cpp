#include "broadPhase.hpp"
#include "collision.hpp"
#include "vector2.hpp"
#include "log.hpp"

#include "game.hpp"
#include "variable.hpp"
#include <angelscript.h>

#include <algorithm>

#include <unordered_set>


void BroadPhase::registerTileCollisionCallback(int meta, TileCollisionCallback* cb)
{

    tileCollisionCallbacks[meta] = cb;
}

void BroadPhase::setTilemap(DefVector2 tsize, TilemapLayer* map)
{
    
    tilemapTileSize = tsize;
    tilemap = ReferenceHolder<TilemapLayer>::create(map);
    
}
    
void BroadPhase::init()
{
    if (initialized)
        throw std::runtime_error("BroadPhase init called when already initialized");
    pam = std::unique_ptr<PhysicsActorManager>(new PhysicsActorManager(engine->getScriptEngine()));
    initialized = true;
}

void BroadPhase::deInit()
{
    if (!initialized)
        throw std::runtime_error("BroadPhase deInit called when already deinitialized");
    clear();
    pam.reset();
    initialized = false;
}

void BroadPhase::scrLinkActor(void* ptr, int tid)
{
    auto* p = pam->checkIsValidPhysicsActor(ptr,tid);
    if (!p)
    {
        asGetActiveContext()->SetException("RegisterCollision called with illegal arguments");
        return;
    }
    linkActor(p);
}
void BroadPhase::scrRemoveActor(void* ptr, int tid)
{
    auto* p = pam->checkIsValidPhysicsActor(ptr,tid);
    if (!p)
    {
        asGetActiveContext()->SetException("ReleaseCollision called with illegal arguments");
        return;
    }
    removeActor(p);
}

void BroadPhase::clear()
{
    if (!initialized)
        return;

    for (auto& a : actors)
    {
        pam->releaseActor(a.first);
        delete a.second;
    }
    actors.clear();
    for (auto* a : physicsActorsToBeAdded)
        pam->releaseActor(a);
    physicsActorsToBeAdded.clear();
    physicsActorsToBeRemoved.clear();

    quadTree.clear();
}

void BroadPhase::linkActor(PhysicsActorType* a)
{
    if (a == nullptr)
        return;

    pam->addActorRef(a);
    physicsActorsToBeAdded.push_back(a);
}

void BroadPhase::removeActor(PhysicsActorType* a)
{
    if (a == nullptr)
        return;

    physicsActorsToBeRemoved.push_back(a);
}

BroadPhase::BroadPhase(Engine* e)
: quadTree(512, 4, 4)
{
    engine = e;

}

BroadPhase::~BroadPhase()
{
    for (auto it: actors)
        delete it.second;
    if (initialized)
        deInit();
}


void BroadPhase::narrowPhase(CollisionEntity* aent, CollisionEntity* bent)
{
    auto a = aent->actor;
    auto b = bent->actor;
    int aflags,awith,atype;
    int bflags,bwith,btype;

    PhysicsActorData apad;
    pam->getActorData(a, &apad);
    PhysicsActorData bpad;
    pam->getActorData(b, &bpad);

    aflags = apad.cflags; awith = apad.with; atype = apad.type;
    bflags = bpad.cflags; bwith = bpad.with; btype = bpad.type;


    if (!((awith & btype)||(bwith & atype)))
        return;

    if (bflags&COLLISION_IS_PROJECTILE)
    {
        BoxLineCollision bc = BoxLineCollision::FindCollision(apad.position, apad.size, bpad.position, bent->lastPos);
        if (bc.found)
        {
            pam->collideActorWith(b, a, ActorCollisionInfo());
        }
    }
    else if (aflags&COLLISION_IS_PROJECTILE)
    {
        BoxLineCollision bc = BoxLineCollision::FindCollision(bpad.position, bpad.size, apad.position, aent->lastPos);
        if (bc.found)
        {
            pam->collideActorWith(a, b, ActorCollisionInfo());
        }
    }
    else
    {

        BoxSweepCollision d = BoxSweepCollision::FindCollisionSlide(aent->lastPos, aent->targetPos, apad.size, bent->lastPos, bent->targetPos, bpad.size, aflags&COLLISION_IS_STATIC, bflags&COLLISION_IS_STATIC);
        if (d.found)
        {
            if (!((aflags&COLLISION_IS_GHOST) || (bflags&COLLISION_IS_GHOST)))
            {
                if (aflags&COLLISION_IS_STATIC)
                {
                    if (bflags&COLLISION_IS_STATIC)
                    {

                    }
                    else
                    {
                        pam->setActorPosition(b, d.o2NewCenter);
                        bent->targetPos = d.o2NewCenter;
                    }
                }
                else
                {
                    if (bflags&COLLISION_IS_STATIC)
                    {
                        pam->setActorPosition(a, d.o1NewCenter);
                        aent->targetPos = d.o1NewCenter;
                    }
                    else
                    {
                        pam->setActorPosition(a, d.o1NewCenter);
                        pam->setActorPosition(b, d.o2NewCenter);
                        aent->targetPos = d.o1NewCenter;
                        bent->targetPos = d.o2NewCenter;
                    }
                }
            }
            ActorCollisionInfo aci;
            aci.fix = d.o1NewCenter - aent->targetPos;
            aci.normal = (d.o1Normal);

            if (aflags&COLLISION_CALLBACK)
                pam->collideActorWith(a, b, aci);

            aci.fix = d.o2NewCenter - bent->targetPos;
            aci.normal *= -1;
            if (bflags&COLLISION_CALLBACK)
                pam->collideActorWith(b, a, aci);
            
        }
    }
}



void BroadPhase::queryBoxCollision(DefVector2 center, DefVector2 size, BoxCollisionQuery* bcq)
{

    DefVector2 min, max;
    min = center - size;
    max = center + size;

    quadTree.areaFind(min, max, [&](BroadPhaseQuadTreeEntity * e)
    {
        PhysicsActor* a = e->entity.actor;

        PhysicsActorData pad;
        pam->getActorData(a, &pad);

        DefVector2 ap = pad.position;
        DefVector2 as = pad.size;

        BoxCollision bc = BoxCollision::FindCollision(center, size, ap, as);

        if (bc.found)
            bcq->collision(a);
    });
}


void BroadPhase::queryCircleCollision(DefVector2 center, float r, CircleCollisionQuery* c)
{

    DefVector2 min, max;
    min.x = center.x-r;
    min.y = center.y-r;
    max.x = center.x+r;
    max.y = center.y+r;

    std::unordered_set<PhysicsActor*> factors;
    quadTree.areaFind(min, max, [&](BroadPhaseQuadTreeEntity * e)
    {
        factors.insert(e->entity.actor);
    });

    for (PhysicsActor* a : factors)
    {
        PhysicsActorData pad;
        pam->getActorData(a, &pad);

        DefVector2 pos = pad.position;
        DefVector2 size = pad.size/2;


        //the circle must be within this rectangle for the collision
        //to be possible
        DefVector2 sizerad = pad.size/2;
        sizerad.x += r;
        sizerad.y += r;

        DefVector2 delta = center-pos;
        delta.x = fabs(delta.x);
        delta.y = fabs(delta.y);

        if (delta.x > sizerad.x)
            continue;

        if (delta.y > sizerad.y)
            continue;

        //delta.x - radius < size.x
        //If the circle is directly above, below, left or right to the
        //box, a collision occurred

        if (delta.x < size.x)
        {
            c->collision(a, delta.length());
            continue;
        }

        if (delta.y < size.y)
        {
            c->collision(a, delta.length());
            continue;
        }

        //Check corners next, need to actually calculate distances

        DefVector2 delta2 = delta-size;
        
        //Compare squared distances for EFFICIENCY
        if (delta2.x*delta2.x+delta2.y*delta2.y < r*r)
        {
            c->collision(a, delta.length());
        }
    }
}





void BroadPhase::queryLineCollision(DefVector2 p1, DefVector2 p2, LineCollisionQuery* c)
{
    DefVector2 min, max;
    min.x = std::min(p1.x,p2.x);
    min.y = std::min(p1.y,p2.y);
    max.x = std::max(p1.x,p2.x);
    max.y = std::max(p1.y,p2.y);

    quadTree.areaFind(min, max, [&](BroadPhaseQuadTreeEntity * e)
    {

        PhysicsActor* a = e->entity.actor;
        PhysicsActorData pad;
        pam->getActorData(a, &pad);
        BoxLineCollision blc = BoxLineCollision::FindCollisionPoint(pad.position, pad.size, p1, p2);
        if (blc.found)
        {
            c->collision(a, blc.point);
        }
    });

    /*
    if (mapHandler)
    {
        Vector2i ret = Vector2i(0,0);
        if (mapHandler->getLineCollisionTile(p1, p2, ret))
        {
            c->collision(nullptr, ret*mapHandler->getTileSize() + mapHandler->getTileSize() / 2);
        }
    }
    */
}


void BroadPhase::insertNewActorToQuadTree(std::pair< PhysicsActor* const, BroadPhaseQuadTreeEntity*>& p)
{
    PhysicsActorData pad;
    pam->getActorData(p.first, &pad);

    DefVector2 hs = pad.size/2;
    DefVector2 pos = pad.position;

    BroadPhaseQuadTreeEntity* cs = new BroadPhaseQuadTreeEntity(pos-hs,pos+hs);
    if (std::isnan(pos.x) || std::isnan(pos.y))
    {
        pos.x = 0.0;
        pos.y = 0.0;
    }
    cs->entity.lastPos = pos;
    cs->entity.targetPos = pos;
    cs->entity.actor = (PhysicsActor*) p.first;
    quadTree.insert(cs);
    p.second = cs;
}

void BroadPhase::setCollisionWorldSize(DefVector2 c)
{
    collisionWorldSize = c;
    if (c.x <= 0 || c.y <= 0)
    {
        active = false;
        return;
    }
    active = true;
    
    quadTree.clear();
    quadTree.create({0,0}, collisionWorldSize);

    for (auto& p : actors)
    {
        auto* b = p.second;
        if (b == nullptr)
        {
            insertNewActorToQuadTree(p);
        }
        else
        {
            quadTree.insert(p.second);
        }
    }
}

void BroadPhase::update()
{
    if (!initialized)
        throw std::runtime_error("BroadPhase update called when not initialized");
    
    
    if (!active)
        return;
    
    for (PhysicsActorType* pat : physicsActorsToBeAdded)
    {
        auto it = actors.find(pat);
        if (it != actors.end())
        {
            pam->releaseActor(pat);
        }
        else
            actors.insert({pat,nullptr});
    }

    for (PhysicsActorType* pat : physicsActorsToBeRemoved)
    {
        auto it = actors.find(pat);
        if (it != actors.end())
        {
            auto* st = it->second;
            if (st)
            {
                quadTree.remove(st);
                delete st;

            }
            pam->releaseActor(it->first);
            actors.erase(it);
        }
    }
    physicsActorsToBeRemoved.clear();
    physicsActorsToBeAdded.clear();

    if (!quadTree.isInitialized())
        quadTree.create({0,0}, collisionWorldSize);


    for (auto& p : actors)
    {
        PhysicsActor* const a = p.first;

        if (p.second == nullptr)
        {
            insertNewActorToQuadTree(p);
        }

        PhysicsActorData pad;
        pam->getActorData(a, &pad);


        int flags = pad.cflags;

        DefVector2 pos = pad.position;
        DefVector2 hs = pad.size/2;
        CollisionEntity& cc = p.second->entity;

        if (std::isnan(cc.targetPos.x) || std::isnan(cc.targetPos.y))
        {
            cc.targetPos.x = 0.0;
            cc.targetPos.y = 0.0;
        }
        if (std::isnan(pos.x) || std::isnan(pos.y))
        {
            pos.x = 0.0;
            pos.y = 0.0;
        }

        cc.lastPos = cc.targetPos;

        if (cc.targetPos != pos)
        {
            cc.targetPos = pos;
            quadTree.remove(p.second);
            if (flags & COLLISION_STEP_TELEPORT)
            {
                cc.lastPos = pos;
                p.second->move(pos-hs,pos+hs);
            }
            else
            {

                //For sweep testing, we need to add the object into quadtree as an AABB containing
                //the whole sweep
                DefVector2 mins = {std::min(pos.x, cc.lastPos.x), std::min(pos.y, cc.lastPos.y)};
                DefVector2 maxs = {std::max(pos.x, cc.lastPos.x), std::max(pos.y, cc.lastPos.y)};
                p.second->move(mins-hs,maxs+hs);
            }
            quadTree.insert(p.second);
        }
    }



    if (tilemap)
    {
        DefVector2 offset = DefVector2(0,0);
        DefVector2 tileSize = tilemapTileSize;
        if (tilemap)
        for (auto it = actors.begin(); it != actors.end(); it++)
        {
            PhysicsActor* const a = it->first;
            CollisionEntity* ent = &it->second->entity;

            PhysicsActorData pad;
            pam->getActorData(a, &pad);

            int aflags,awith,atype;


            aflags = pad.cflags;
            awith = pad.with;
            atype = pad.type;

            (void)atype;

            if (!(awith&COLLIDE_MAP))
                continue;

            if ((aflags&COLLISION_IS_PROJECTILE))
            {
                DefVector2 p1,p2;

                p1 = ent->lastPos;
                p2 = ent->targetPos;
                if (p1 == p2)
                    continue;
                TileMapLineCollision tmlc =  TileMapLineCollision::FindCollision(p1, p2, *tilemap, offset, tileSize);


                if (tmlc.found)
                {
                    pam->setActorPosition(a, tmlc.position+tmlc.normal);
                    pam->collideActorWithStatic(a, MapCollisionInfo());
                }
                continue;
            }

            std::function<DefVector2(Vector2i, DefVector2)> func = [&](Vector2i tl, DefVector2 fix)
            {
                auto t = tilemap->getTile({tl.x, tl.y});
                auto at = tileCollisionCallbacks.find(t);
                if (at != tileCollisionCallbacks.end())
                {
                    return (*at).second->collide(a, tl, fix);
                }
                return fix;
            };

            
            TileMapCollision d = TileMapCollision::FindSweepCollision(ent->lastPos, ent->targetPos, pad.size , *tilemap, offset, tileSize, func);

            if (d.found)
            if (d.fix.x != 0 || d.fix.y != 0)
            {
                DefVector2 ds = d.fix;
                if (ds.length() == 0)
                    continue;

                ent->targetPos = ent->targetPos + d.fix;
                ds = ds/ds.length();
                MapCollisionInfo mci;
                mci.normal = ds;

                pam->setActorPosition(a, ent->targetPos);
                pam->collideActorWithStatic(a, mci);
            }
        }
    }



    int c = 0;
    int collisionPassesCount = engine->getVariableManager()->getIntegerDefault(CHash("Collision.Passes"), 1);
    for (int i = 0; i < collisionPassesCount; i++)
    {
        quadTree.operatePairs([&](BroadPhaseQuadTreeEntity * a, BroadPhaseQuadTreeEntity * b)
        {
            narrowPhase(&a->entity, &b->entity);
            c++;
        });
    }

}
