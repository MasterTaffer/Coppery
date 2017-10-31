
class EventLogicStep
{
    double dt;
}

class EventPlayerStep
{
    double dt;
}

class EventPhysicsStep
{
    double dt;
}

class EventGraphicsStep
{
    double dt;
}

class EventCollideActor
{
    PhysicsActor@ other;
    Vector2 normal;
    Vector2 fix;
}

class EventCollideStatic
{
    Vector2 normal;
    Vector2 point;
}

//This is the Actor
[PhysicsActor,Component]
class PhysicsActor
{
    Entity@ entity;

    //Some properties we want to share on the script side
    /*
        How the collision detector handles this actor.
        Possible flags are within enum CollisionFlags
        Bitwise OR all the desired flags together.
    */
    uint collisionFlags = 0; 

    /*
        In what "collision group" this entity belongs to. Bitwise OR
        of all groups (see CollisionType enum)
    */
    uint collisionType = 0;
    
    /*
        With what collision groups the object will collide. Bitwise OR
        of all groups.
    */
    uint collisionWith = 0;

    Vector2 position = Vector2(0,0);	 	//Actor position
    Vector2 size = Vector2(0,0);			//Collision size.
    Vector2 collisionOffset = Vector2(0,0);//Collision offset. Offsets the actual collision box from the position.
    
    
    //Every actor has an unique ID
    uint id;

    bool collisionActive = false;

    [InitHandler]
    void physicsInit()
    {
        Collision::Add(@this);
        collisionActive = true;
    }
    
    [DeinitHandler]
    void physicsDeinit()
    {
        releaseCollision();
    }


    void releaseCollision()
    {
        if (collisionActive)
            Collision::Remove(@this);
        collisionActive = false;
    }
    //Called when our actor collides with another
    void collide(PhysicsActor@ other, const Collision::ActorCollisionInfo& aci)
    {
        EventCollideActor eac;
        @eac.other = other;
        eac.normal = aci.normal;
        eac.fix = aci.fix;
        ESM::QueueLocalEvent(entity, eac);

    }

    //Called when our actor collides upon
    //the static geometry of the almighty Map
    void collideStatic(const Collision::MapCollisionInfo& mci)
    {
        EventCollideStatic emc;
        emc.normal = mci.normal;
        emc.point = mci.point;
        ESM::QueueLocalEvent(entity, emc);
    }

}

enum CollisionFlags
{
    /*
        Perform a line collision instead of the regular box collision check.
        Size is irrelevant in the line collision checking.
    */
    CollisionIsProjectile = 2,
    /*
        Object is effectively pass through. Use this if you want only the 
        collision callback triggered.
    */
    CollisionGhost = 4,
    /*
        Object is immovable by the collision engine. Other objects will collide
        against this.
    */
    CollisionStatic = 8,
    /*
        Call the collide & collideStatic methods upon collisions.
    */
    CollisionCallback = 16,
    
    /*
        "Teleport" the object: do not perform sweep checks.
        The collision system always assumes that the object moves
        from the previous position to the new position in a linear fashion,
        unless this flag is set.
        
        Useful when the object "teleports" from a place to another.
    */
    CollisionStepTeleport = 32
}

enum CollisionType
{
    CollisionPlayer = 1,
    CollisionMap = 2,
    CollisionStar = 4,
    CollisionBullet = 8
}

