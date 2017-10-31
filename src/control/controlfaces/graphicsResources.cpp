
#include "graphics/graphics.hpp"
#include "graphics/shader.hpp"
#include "graphics/graphicsData.hpp"
#include "graphicsResources.hpp"
#include "fileOperations.hpp"

Shader* createShader(Graphics* graphics, const char* name, const char* fragment, const char* vertex, const char* geometry)
{
    if (!name)
        return nullptr;
    Shader* dshader = graphics->getAssets()->shaders.addElement(Hash(name));

    
    if (fragment && *fragment != 0)
        dshader->addFragShader(AssetsFolder+"/shaders/"+fragment);
    if (vertex && *vertex != 0)
        dshader->addVertShader(AssetsFolder+"/shaders/"+vertex);
    if (geometry && *geometry != 0)
        dshader->addGeomShader(AssetsFolder+"/shaders/"+geometry);
    return dshader;
}

Shader* getShader(Graphics* graphics, const char* str)
{
    if (str)
        return graphics->getAssets()->shaders.getElement(Hash(str));
    return nullptr;
}


Texture* getTexture(Graphics* graphics, const char* str)
{
    if (str)
        return graphics->getAssets()->textures.getElement(Hash(str));
    return nullptr;
}


Font* getFont(Graphics* graphics, const char* str)
{
    if (str)
        return graphics->getAssets()->fonts.getElement(Hash(str));
    return nullptr;
}


int getShaderUniform(Graphics* graphics, Shader* shader, const char* uniform)
{
    if (shader && uniform)
    {
        return shader->getUniform(Hash(uniform));
    }
    return -1;
}

void loadShaders(Graphics* graphics)
{
    graphics->getAssets()->shaders.init();
}

luaL_Reg graphicsResources_functions[] =
{
    {"CreateShader", LuaClosureWrap(createShader, 1)},
    {"GetShader", LuaClosureWrap(getShader, 1)},
    {"GetShaderUniform", LuaClosureWrap(getShaderUniform, 1)},
    {"LoadShaders", LuaClosureWrap(loadShaders, 1)},
    {"GetFont", LuaClosureWrap(getFont, 1)},
    {"GetTexture", LuaClosureWrap(getTexture, 1)},
    {0,0}
};
