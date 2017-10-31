


Sound@ Player_Shoot = GetSound(Hash("shoot"));
[Component]
class PlayerComponent
{
    Entity@ entity;
    
    [ComponentRef]
    PhysicsActor@ body;
    [ComponentRef]
    AttractedComponent@ attracted;
    [ComponentRef]
    SpriteComponent@ sprite;
    
    double rot;
    double repeatFire = 0.0;
    [InitHandler]
    void init()
    {
        body.collisionType = CollisionPlayer;
        body.collisionWith = CollisionStar;
        body.collisionFlags = CollisionCallback;
        body.size = Vector2(16,16);
        sprite.sprite.setTexture(Hash("arrow"));
        rot = 0;
    }
    
    [EventHandler]
    void collide(const EventCollideActor &in e)
    {
        KillPlayerComponent@ kpc;
        e.other.entity.getComponent(@kpc);
        if (kpc !is null)
            kill();
    }
    
    void kill()
    {
        
        explode(body.position);
        ESM::KillEntity(entity);
    }
    
    [EventHandler]
    void updatePlayer(const EventPlayerStep &in e)
    {
        
        double move = GetInput(Hash("move_horizontal")).getValue();
        double moveVert = 0 - GetInput(Hash("move_vertical")).getValue();
        rot += move * e.dt * 4;
        
        double pi = 3.1415925;
        double rotrad = rot * pi / 180.0;
        Vector2 a = Vector2(cos(rotrad), sin(rotrad));
        
        attracted.velocity += a * moveVert * e.dt * 0.2;
        
        if (GetInput(Hash("attack1")).getValue() != 0 && repeatFire <= 0)
        {
            BulletComponent@ ac;
            ESM::ConstructEntity(EM_Bullet).getComponent(@ac);
            ac.body.position = body.position;
            ac.velocity = a*8;
            Player_Shoot.play();
            repeatFire = 8.0;
        }
        repeatFire -= e.dt;
        
    }
    
    [EventHandler]
    void updateGraphics(const EventGraphicsStep &in e)
    {
        sprite.sprite.setRotation(rot);
    }
    
    [DeinitHandler]
    void deinit()
    {
        PlayerDie();
        
    }
}


EntityMold@ EM_Player = {
    ComponentInfo<PlayerComponent>().getId(),
    ComponentInfo<AttractedComponent>().getId(),
    ComponentInfo<SpriteComponent>().getId(),
    ComponentInfo<PhysicsActor>().getId()
};

bool Main_LoadNewMap = false;

DrawText Main_DebugText;

[OnInit]
void MainInit()
{
    GetSound(Hash("music")).play();
    Main_DebugText.register(DrawableLayerLightlessMesh);
    Main_DebugText.setDepth(-79);
    Main_DebugText.setFont(Hash("fontMain"));
    Main_DebugText.setPosition(Vector2(2,8));
    Main_DebugText.setScale(2);
    
    AddEndStepCallback(MainLoop);
    ResetMainState();
}

void explodeSmall(Vector2 position)
{

    hash_t psys = Hash("sparks");
    SpawnParticle(psys, position, Vector2(0,0), Color(), 10);    
}

void explode(Vector2 position)
{

    hash_t psys = Hash("sparks");
    for (uint i = 0; i < 125; i++)
    SpawnParticle(psys, position, Vector2(GetRandom()*8-4,GetRandom()*8-4), Color(), 1);    
}

int Main_Phase = 0;
int Main_RocksDestroyed = 0;
int Main_RocksSpawned = 0;
int Main_RocksToSpawn = 0;
int Highscore = 0;


void ResetMainState()
{
    ESM::LogDebugInfo();
    ESM::KillAllEntities();
    ESM::UpdateEntityLists();

    Collision::RemoveAll();
    PhysicsActor@ p;
    WorldSize = Vector2(700, 700);

    ESM::ConstructEntity(EM_Player).getComponent(@p);
    p.position = WorldSize / 8;
    
    AttractorComponent@ ac;
    ESM::ConstructEntity(EM_Star).getComponent(@ac);
    ac.body.position = WorldSize / 2;
    ac.attracted.velocity = Vector2(0, 1);
    ac.mass=3;
    
    ESM::ConstructEntity(EM_Star).getComponent(@ac);
    ac.body.position = Vector2(WorldSize.x / 3, WorldSize.y * 3 / 5);
    ac.attracted.velocity = Vector2(0, -1);
    ac.mass=3;
    
    
    Collision::SetWorldSize(WorldSize);
    
    Main_RocksDestroyed = 0;
    Main_Phase = 0;
    Main_RocksSpawned= 0;
    Main_RocksToSpawn = 0;
    PlayerDead = false;
    LoadHighscore();
    
}


bool PlayerDead = false;
void PlayerDie()
{
    Main_Splat.play();
    PlayerDead = true;
    if (Highscore < Main_RocksDestroyed)
    {
        Highscore = Main_RocksDestroyed;
        DataTable@ dt = DataTable::Create();
        dt.setInt("highscore", Highscore);
        DataTable::Save(dt, "user/data/highscore.xml");
    }
}

void LoadHighscore()
{
    DataTable@ dt = DataTable::Load("user/data/highscore.xml");
    if (dt !is null)
    {
        dt.getInt("highscore", Highscore);
    }
}


Sound@ Main_WaveComplete = GetSound(Hash("beep1"));
Sound@ Main_WaveSpawn = GetSound(Hash("effect1"));
Sound@ Main_Splat = GetSound(Hash("splat"));
Vector2 WorldSize;

bool GameActive = false;

bool MainLoop()
{
    
    ConstrainCameraPosition();
        
    if (GameActive)
    {
        if (GetIsMainStep())
        {
            
            double diff = Main_RocksDestroyed;
            if (Main_Phase > 0)
            {
            
                if (Main_Phase > 100)
                {
                    Main_Phase -= int(2500.0 / (50 + diff));
                    Main_RocksToSpawn -= 1;
                    Main_RocksSpawned += 1;
                    Main_WaveSpawn.play();
                    
                    PhysicsActor@ p;
                    ESM::ConstructEntity(EM_Rock).getComponent(@p);
                    if (GetRandom() > 0.5)
                        p.position = Vector2(WorldSize.x, GetRandom()*WorldSize.y);
                    else
                        p.position = Vector2(0, GetRandom()*WorldSize.y);
                
                    if (Main_RocksToSpawn <= 0)
                    {
                        Main_Phase = 0;
                        Main_RocksToSpawn = 0;
                    }
                }
                else
                    Main_Phase ++;
            }
            else
            if (Main_RocksSpawned <= Main_RocksDestroyed)
            {
                Main_WaveComplete.play();
                Main_RocksToSpawn = Main_RocksDestroyed / 4 + 2;
                Main_Phase = 1;
            }
        }
        
        while (ESM::SendEvents())
            (void);

        ESM::UpdateEntityLists();

        double dt = GetRealDeltaTime();
        
        
        {
            EventPlayerStep eps;
            eps.dt = dt;
            ESM::QueueGlobalEvent(eps);
            ESM::SendEvents();
        }
        
        {
            EventLogicStep els;
            els.dt = dt;
            ESM::QueueGlobalEvent(els);
            ESM::SendEvents();
        }
        
        ESM::SendEvents();

        {
            EventPhysicsStep eps;
            eps.dt = dt;
            ESM::QueueGlobalEvent(eps);
            ESM::SendEvents();
        }

        ESM::UpdateEntityLists();
        
        Collision::RunDetection();

        ESM::SendEvents();


        {
            EventGraphicsStep egs;
            egs.dt = dt;
            ESM::QueueGlobalEvent(egs);
        }

        while (ESM::SendEvents())
            (void);
        FormatString fs;
        if (!PlayerDead)
        {
            fs.compile("Highscore: %  Score: %");
            FormatString::Formatter fsf;
        
            fsf.add(Highscore);
            fsf.add(Main_RocksDestroyed);
            
            string outstr;
            fsf.format(fs, outstr);
            Main_DebugText.setText(outstr);
        }
        else
        {
            fs.compile("Highscore: %\nPress 'i' to restart");
            FormatString::Formatter fsf;
        
            fsf.add(Highscore);
            
            
            string outstr;
            fsf.format(fs, outstr);
            Main_DebugText.setText(outstr);
        }
    }
    else
    {  
        Main_DebugText.setText("ART THOU READY?\n\nControl thy ship with WASD\nSmite enemy with RIGHT CTRL\nAvoid the WHITE THINGS\n\nFor too long evil hath reign'd\n...\nPRESS RIGHT CTRL TO BATTLE EVIL");
        if (GetInput(Hash("attack1")).getPressed() != 0)
            GameActive = true;
    
    }
        
  
    
    return false;
}




uint DefaultInputLayer = 2;


Input@ GetInput(hash_t h)
{
    return Input::GetInput(h, DefaultInputLayer);
}

