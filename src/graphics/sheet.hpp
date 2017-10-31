#pragma once
#include "elementMapper.hpp"
#include <string>
#include "vector2.hpp"

class GraphicsData;
class Texture;
class TextureSheet
{
    void load(GraphicsData*);
    Vector2i splits;
    Texture* texture;
    void unLoad();
    Vector2i baseSize;
    std::vector<std::string> toAdd;
    std::vector<std::string> toAddNames;
    std::unordered_map<Hash, int> nameMap;
    friend class DrawableTilemap;
public:

    Vector2f getUVSize();

    Vector2f getUVBase(int);

    TextureSheet* setBaseSize(Vector2i i)
    {
        baseSize = i;
        return this;
    }

    Texture* getTexture();

    TextureSheet* addFile(std::string f, std::string name)
    {
        toAdd.push_back(f);
        toAddNames.push_back(name);
        return this;
    }

    int getIndexByName(Hash h)
    {
        auto it = nameMap.find(h);
        if (it != nameMap.end())
            return (*it).second;
        return 0;
    }

    friend class TextureSheetMap;
};

class TextureSheetMap : public ElementMapper<TextureSheet>
{
    GraphicsData* assets = nullptr;
public:

    void setAssets(GraphicsData* g) {assets = g;};

protected:
    void initializeElement(TextureSheet*);
    void deInitializeElement(TextureSheet*);
};

