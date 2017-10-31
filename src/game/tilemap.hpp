#pragma once
#include <vector>
#include <unordered_map>
#include "vector2.hpp"
#include "hash.hpp"
#include "mapObject.hpp"
#include "reference.hpp"


//! Collection of named tiledefinitions
class Tileset
{
    MixinReferenceCounted
public:
    //! Key used 
    using Key = Hash;
    //! Value used
    using Value = int;
    
private:
    static constexpr const Value nullValue = 0;
    
public:
    
    //! Add a new key to the set
    Value addDef(Key k)
    {
        auto it = defs.find(k);
        if (it == defs.end())
        {
            Value now = next;
            ++next;
            defs[k] = now;
            return now;
        }
        return it->second;
    }
    
    //! Get the value corresponding to key
    Value getDef(Key k) const
    {
        auto it = defs.find(k);
        if (it == defs.end())
            return nullValue;
        return it->second;
    }
    
    //! Get the greates value in the tileset
    Value maximumValue() const
    {
        return next - 1;
    }
    
    //! Gets raw access to defs
    const std::unordered_map<Key, Value>& getDefs() const
    {
        return defs;
    }
private:
    Value next = 1;
    std::unordered_map<Key, Value> defs;
    friend class DrawableTilemap;
};

//! Single tilemap
class TilemapLayer
{
    MixinReferenceCounted
    
    std::vector<Tileset::Value> data;
    int width = 0, height = 0;
    Tileset::Value minimum = 0, maximum = 0;
    bool tilesetValidated = false;
    ReferenceHolder<Tileset> tileset;
    
    friend class DrawableTilemap;
public:
    //! Clears the map to 0
    void clear();
    //! Get the size of the map
    Vector2i getSize();
    //! Resize the map
    void resize(int width, int height);
    //! Validate the map for tileset transformation
    bool validate();
    //! Set tile
    void set(Vector2i pos, Tileset::Value);
    //! Fill rectangular area
    void fill(Vector2i min, Vector2i to, Tileset::Value);
    //! Set tileset used
    void setTileset(Tileset*);
    /*! \brief Load map data in bulk
     * 
     * ptr must be a pointer to an array with at least height times width
     * elements.
     */
    template <typename T>
    void loadBulk(T* ptr)
    {
        minimum = 0;
        maximum = 0;
        for (auto& v : data)
        {
            v = *ptr;
            if (*ptr < minimum)
                minimum = *ptr;
            if (*ptr > maximum)
                maximum = *ptr;
            ++ptr;
        }
    }
    
    //! Get tile at position
    int getTile(Vector2i pos)
    {
        if (pos.x < 0 || pos.x >= width)
            return 0;
        if (pos.y < 0 || pos.y >= height)
            return 0;
        return data[pos.x + pos.y * width];
    }
};

//! Collection holding multiple TilemapLayers and MapObjects
class Tilemap
{
    MixinReferenceCounted
    
    std::unordered_map<Hash, ReferenceHolder<TilemapLayer>> layers;
    std::vector<ReferenceHolder<MapObject>> objects;
public:
    //! Adds a new layer 
    void addLayer(Hash name, TilemapLayer* layer)
    {
        layers[name] = ReferenceHolder<TilemapLayer>::create(layer);
    }
    
    /*! \brief Gets a layer by name.
     * 
     * Increases the reference count of the returned layer.
     */
    TilemapLayer* getLayer(Hash name)
    {
        auto it = layers.find(name);
        if (it == layers.end())
            return nullptr;
        it->second->addRef();
        return it->second.get();
    }
    
    
    //! Adds a new MapObject
    void addObject(MapObject* object)
    {
        objects.push_back(ReferenceHolder<MapObject>::create(object));
    }
    
    /*! \brief Gets a MapObject by index.
     * 
     * Increases the reference count of the returned MapObject.
     */
    MapObject* getObject(size_t index)
    {
        if (index >= objects.size())
            return nullptr;
        objects[index]->addRef();
        return objects[index].get();
    }
    
    
    //! Get the count of MapObject instances in the map
    size_t getObjectCount()
    {
        return objects.size();
    }
    
};
