
[Component]
class BulletComponent
{
    Entity@ entity;
    [ComponentRef]
    PhysicsActor@ body;
    [ComponentRef]
    SpriteComponent@ sprite;
    
    
    [InitHandler]
    void init()
    {
        body.collisionFlags = CollisionCallback;
        
        body.collisionType = CollisionBullet;
        body.size = Vector2(8,8);
        sprite.sprite.setTexture(Hash("bullet"));
    }
      
    Vector2 velocity = Vector2(0,0);
    
    [EventHandler]
    void collide(const EventCollideActor &in e)
    {
        RockComponent@ rc;
        e.other.entity.getComponent(@rc);
        if (rc !is null)
        {
            rc.damage();
            kill();
        }
    }
    
    [EventHandler]
    void updatePhysics(const EventPhysicsStep &in e)
    {
        body.position += velocity;
        if (body.position.x < 0)
        {
            kill();
        }
        if (body.position.x > WorldSize.x)
        {
            kill();
        }
        if (body.position.y < 0)
        {
            kill();
        }
        if (body.position.y > WorldSize.y)
        {
            kill();
        }
    }
    
    void kill()
    {
        explodeSmall(body.position);
        ESM::KillEntity(entity);
    }
}


EntityMold@ EM_Bullet = {
    ComponentInfo<BulletComponent>().getId(),
    ComponentInfo<SpriteComponent>().getId(),
    ComponentInfo<PhysicsActor>().getId()
};


