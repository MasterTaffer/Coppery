
Sound@ Rock_Boom = GetSound(Hash("boom"));

[Component]
class RockComponent
{
    Entity@ entity;
    [ComponentRef]
    PhysicsActor@ body;
    [ComponentRef]
    SpriteComponent@ sprite;
    
    double rot;
    int health = 1;
    [InitHandler]
    void init()
    {
        body.collisionType = CollisionStar;
        body.collisionWith = CollisionBullet;
        body.size = Vector2(8,8);
        sprite.sprite.setTexture(Hash("rock"));
        rot = GetRandom()*360;
    }
    
    [EventHandler]
    void updateGraphics(const EventGraphicsStep &in e)
    {
        rot += e.dt;
        sprite.sprite.setRotation(rot);
    }
    
    void damage()
    {
        health -= 1;
        if (health == 0)
        {
            Rock_Boom.play();
            ESM::KillEntity(entity);
            Main_RocksDestroyed += 1;
        }
    }
    
}


EntityMold@ EM_Rock = {
    ComponentInfo<KillPlayerComponent>().getId(),
    ComponentInfo<AttractedComponent>().getId(),
    ComponentInfo<SpriteComponent>().getId(),
    ComponentInfo<RockComponent>().getId(),
    ComponentInfo<PhysicsActor>().getId()
};

