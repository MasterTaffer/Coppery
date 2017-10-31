
[Component]
class KillPlayerComponent
{
}

[Component]
class SpriteComponent
{
    Entity@ entity;
    
    Sprite sprite;
    
    [ComponentRef]
    PhysicsActor@ body;
    
    [InitHandler]
    void init()
    {
        sprite.register();
    }
    
    [EventHandler]
    void updateGraphics(const EventGraphicsStep &in e)
    {
        sprite.setPosition(body.position);
    }
    
    [DeinitHandler]
    void deinit()
    {
        sprite.release();
    }
}

class EventGraviton
{
    double dt;
    Vector2 origin;
    double mass;
}

[Component]
class AttractedComponent
{
    [ComponentRef]
    PhysicsActor@ body;
    [ComponentRef]
    AttractorComponent@ attractor;
    
    Vector2 velocity = Vector2(0,0);
    
    
    [EventHandler]
    void updatePhysics(const EventPhysicsStep &in e)
    {
        const double bounceDrag = 0.4;
        body.position += velocity;
        if (body.position.x < 0)
        {
            body.position.x = 0;
            velocity.x *= -1;
            velocity *= bounceDrag;
        }
        if (body.position.x > WorldSize.x)
        {
            body.position.x = WorldSize.x;
            velocity.x *= -1;
            velocity *= bounceDrag;
        }
        if (body.position.y < 0)
        {
            body.position.y = 0;
            velocity.y *= -1;
            velocity *= bounceDrag;
        }
        if (body.position.y > WorldSize.y)
        {
            body.position.y = WorldSize.y;
            velocity.y *= -1;
            velocity *= bounceDrag;
        }
    }
    
    [EventHandler]
    void receiveGraviton(const EventGraviton &in e)
    {
        Vector2 diff = e.origin - body.position;
        double len = diff.length();
        if (len > 0)
        {
            double multiplier = e.mass;
            //If the entity is an attractor, use own mass as well
            if (attractor !is null)
                multiplier /= attractor.mass;
            velocity += diff * multiplier * e.dt / (len * len);
        }
    }
}

[Component]
class AttractorComponent
{
    [ComponentRef]
    AttractedComponent@ attracted;
    [ComponentRef]
    PhysicsActor@ body;
    double mass = 1.0;
    
    
    [EventHandler]
    void updateLogic(const EventLogicStep &in e)
    {
        EventGraviton eg;
        eg.dt = e.dt;
        eg.origin = body.position;
        eg.mass = mass;
        ESM::QueueGlobalEvent(eg);
    }
}
