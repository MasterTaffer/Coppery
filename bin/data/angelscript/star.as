

[Component]
class StarComponent
{
    [ComponentRef]
    PhysicsActor@ body;
    [ComponentRef]
    SpriteComponent@ sprite;
    
    double rot;
    [InitHandler]
    void init()
    {
        body.collisionType = CollisionStar;
        body.size = Vector2(16,16);
        sprite.sprite.setTexture(Hash("star"));
        rot = GetRandom()*360;
    }
    
    [EventHandler]
    void updateGraphics(const EventGraphicsStep &in e)
    {
        rot += e.dt / 2.0;
        sprite.sprite.setRotation(rot);
    }
    
}


EntityMold@ EM_Star = {
    ComponentInfo<KillPlayerComponent>().getId(),
    ComponentInfo<AttractorComponent>().getId(),
    ComponentInfo<AttractedComponent>().getId(),
    ComponentInfo<SpriteComponent>().getId(),
    ComponentInfo<StarComponent>().getId(),
    ComponentInfo<PhysicsActor>().getId()
};

