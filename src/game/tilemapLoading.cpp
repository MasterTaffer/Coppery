
#define GMS_USE_NAMESPACE
#define GMS_CREATE_IMPLEMENTATION
#include "gameMapSaving.hpp"

#include "fileOperations.hpp"
#include "fileStreambuf.hpp"
#include "game/tilemap.hpp"
#include "game/tilemapLoading.hpp"



Tilemap* TilemapLoading::LoadTSMap(const std::string& fn)
{
    GMS::MapData md;
    
    try
    {
        auto f = GetFileStream(fn);
        if (!f)
            return nullptr;
     
        auto m = std::make_unique<FileStreambuf>(std::move(f));
        std::istream istream(m.get());
        
        GMS::LoadMap(md, istream);
    }
    catch (...)
    {
        return nullptr;
    }
    
    if (md.failed)
        return nullptr;
    
    auto set = ReferenceHolder<Tileset>::construct();
    for (int i = 1; i < md.tileNames.size(); i++)
        set->addDef(Hash(md.tileNames[i]));
    
    auto fg = ReferenceHolder<TilemapLayer>::construct();
    fg->setTileset(set.get());
    fg->resize(md.width, md.height);
    fg->loadBulk(md.fg.data());
    
    auto bg = ReferenceHolder<TilemapLayer>::construct();
    bg->setTileset(set.get());
    bg->resize(md.width, md.height);
    bg->loadBulk(md.bg.data());
    
    auto tfg = ReferenceHolder<TilemapLayer>::construct();
    tfg->setTileset(set.get());
    tfg->resize(md.width, md.height);
    tfg->loadBulk(md.tfg.data());
    
    auto meta = ReferenceHolder<TilemapLayer>::construct();
    meta->resize(md.width, md.height);
    
    //Map format defines: if fg[x] is non 0 and collsion[x] is zero,
    //collision[x] gets set to 1
    for (size_t i = 0; i < md.collision.size(); i++)
    {
        if (md.collision[i] == 0)
            if (md.fg[i] != 0)
                md.collision[i] = 1;
            
    }
    meta->loadBulk(md.collision.data());
    
    Tilemap* tm = new Tilemap();
    
    tm->addLayer(CHash("fg"), fg.get());
    tm->addLayer(CHash("bg"), bg.get());
    tm->addLayer(CHash("tfg"), tfg.get());
    tm->addLayer(CHash("meta"), meta.get());
    
    for (auto& o : md.objects)
    {
        auto obj = ReferenceHolder<MapObject>::construct();
        obj->x = o.x;
        obj->y = o.y;
        obj->name = std::move(o.name);
        obj->type = std::move(o.type);
        obj->properties = std::move(o.properties);
        
        tm->addObject(obj.get());
    }
    return tm;
}
