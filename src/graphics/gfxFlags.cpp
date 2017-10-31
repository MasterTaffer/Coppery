#include "gfxFlags.hpp"
#include "config.hpp"
#include "variable.hpp"

void GFXFlags::updateShaderPrefix()
{
    shaderPrefix.clear();
    if (specular)
        shaderPrefix.push_back("#define GFXS_Specular 1");
    else
        shaderPrefix.push_back("#define GFXS_Specular 0");
    if (normal)
        shaderPrefix.push_back("#define GFXS_Normal 1");
    else
        shaderPrefix.push_back("#define GFXS_Normal 0");
    if (fov)
        shaderPrefix.push_back("#define GFXS_FOV 1");
    else
        shaderPrefix.push_back("#define GFXS_FOV 0");
}

void GFXFlags::loadSettings(GameVariableManager* var)
{
    std::vector<GameVariable*> gvars;

    gvars.push_back(var->makeIntegerLimits("Graphics.NormalMapping", 0, 1, 0, std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            normal = gv.getInteger();
            return true;
        })));
    gvars.push_back(var->makeIntegerLimits("Graphics.SpecularLighting", 0, 1, 0, std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            specular = gv.getInteger();
            return true;
        })));

    gvars.push_back(var->makeIntegerLimits("Graphics.PostProcess", 0, 1, 0, std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            postProcess = gv.getInteger();
            return true;
        })));

    gvars.push_back(var->makeIntegerLimits("Graphics.Particles", 1, 32, 1, std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            particles = gv.getInteger();
            return true;
        })));

/*
    dataFloatNames.clear();
    dataFloatNames.push_back("MapBrightnessFG");
    dataFloatNames.push_back("MapBrightnessBG");

    for (auto a : dataFloatNames)
    {
        dataFloat[Hash(a)] = c->getFloat(a,0.0);
        auto hash = Hash(a);
        gvars.push_back(var->makeNumber(std::string("Graphics.") + a, 0.0, std::function<bool(const GameVariable&)>([this, hash](const GameVariable& gv)
        {
            dataFloat[hash] = gv.getNumber();
            return true;
        })));
    }

*/
    var->loadFromConfig(gvars, "graphics");
}