#include "graphics/graphics.hpp"
#include "graphics/configurableParticleSystem.hpp"
#include "graphics/graphicsData.hpp"
#include "graphics/shader.hpp"
#include "fileOperations.hpp"

#include <luawrap.hpp>

#include <array>
#include <functional>
#include <log.hpp>


bool luaGetInteger(lua_State* lua, int idx, const char* name, int& out)
{
    lua_pushstring(lua, name);
    lua_gettable(lua, idx);

    if (!lua_isinteger(lua, -1))
    {
        lua_pop(lua,1);
        std::string fm = name;
        fm += " is not an integer";
        luaL_error(lua, fm.c_str());
        return false;
    }

    out = lua_tointeger(lua, -1);
    lua_pop(lua, 1);

    return true;
}

bool luaGetString(lua_State* lua, int idx, const char* name, std::string& out)
{
    lua_pushstring(lua, name);
    lua_gettable(lua, idx);

    if (!lua_isstring(lua, -1))
    {
        lua_pop(lua,1);
        std::string fm = name;
        fm += " is not a string";

        luaL_error(lua, fm.c_str());
        return false;
    }

    out = lua_tostring(lua, -1);
    lua_pop(lua, 1);

    return true;
}

std::array<std::string, 3> luaGetShaderTriplet(lua_State* lua)
{
    std::array<std::string, 3> vec;


    lua_pushstring(lua, "fragment");
    lua_gettable(lua, -2);
    if (lua_isstring(lua, -1))
    {
        vec[0] = lua_tostring(lua, -1);
    }
    lua_pop(lua, 1);

    lua_pushstring(lua, "vertex");
    lua_gettable(lua, -2);
    if (lua_isstring(lua, -1))
    {
        vec[1] = lua_tostring(lua, -1);
    }
    lua_pop(lua, 1);

    lua_pushstring(lua, "geometry");
    lua_gettable(lua, -2);
    if (lua_isstring(lua, -1))
    {
        vec[2] = lua_tostring(lua, -1);
    }
    lua_pop(lua, 1);
    return vec;
}

void iterateLuaArray(lua_State* lua, std::function<void()> cb)
{
    int i = 1;
    while (true)
    {
        lua_pushinteger(lua, i);
        lua_gettable(lua, -2);

        if (lua_isnil(lua, -1))
        {
            lua_pop(lua, 1);
            break;
        }
        cb();
        lua_pop(lua, 1);
        i += 1;
    }
}




int configurableParticleSystemRegister(lua_State* lua)
{
    Graphics* g = (Graphics*)lua_touserdata(lua, lua_upvalueindex(1));
    if (g == nullptr)
        return 0;

    if (!lua_istable(lua, 1))
    {
        luaL_argerror(lua, 1, "Expected table");
        return 0;
    }

    int layer, drawtype;
    luaGetInteger(lua, 1 ,"layer", layer);
    luaGetInteger(lua, 1 ,"drawType", drawtype);


    std::string name;
    luaGetString(lua, 1 ,"name", name);

    lua_pushstring(lua, "updateShader");
    lua_gettable(lua, 1);

    if (!lua_istable(lua, -1))
    {
        lua_pop(lua,1);
        luaL_argerror(lua, 1, "updateShader is not a table");

    }
    auto updateShader = luaGetShaderTriplet(lua);
    updateShader[0] = "dummy.fs";

    lua_pop(lua,1);


    lua_pushstring(lua, "particleLayout");
    lua_gettable(lua, 1);

    if (!lua_istable(lua, -1))
    {
        lua_pop(lua,1);
        luaL_argerror(lua, 1, "particleLayout is not a table");
    }

    std::vector<ConfigurableParticleLayout> cpls;
    int offset = 0;
    iterateLuaArray(lua, [&]()
    {
        ConfigurableParticleLayout cpl;

        lua_pushstring(lua, "size");
        lua_gettable(lua, -2);
        if (lua_isinteger(lua, -1))
            cpl.size = lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        cpl.offset = offset;
        offset += cpl.size;

        lua_pushstring(lua, "binding");
        lua_gettable(lua, -2);
        if (lua_isinteger(lua, -1))
            cpl.binding = (ConfigurableParticleLayout::Binding)lua_tointeger(lua, -1);
        lua_pop(lua, 1);

        lua_pushstring(lua, "constantValue");
        lua_gettable(lua, -2);
        if (lua_isnumber(lua, -1))
            cpl.constantValue = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_pushstring(lua, "randomMin");
        lua_gettable(lua, -2);
        if (lua_isnumber(lua, -1))
            cpl.randomMin = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        lua_pushstring(lua, "randomMax");
        lua_gettable(lua, -2);
        if (lua_isnumber(lua, -1))
            cpl.randomMax = lua_tonumber(lua, -1);
        lua_pop(lua, 1);

        cpls.push_back(cpl);

    });

    lua_pop(lua,1);


    /*
        DrawLayout
    */
    std::vector<int> drawLayout;

    lua_pushstring(lua, "drawLayout");
    lua_gettable(lua, 1);

    if (lua_istable(lua, -1))
    {

        iterateLuaArray(lua, [&]()
        {
            int i = lua_tointeger(lua, -1);
            if (i < 0 || size_t(i) >= cpls.size())
                i = -1;
            drawLayout.push_back(i);
        });
    }
    else
    {
        drawLayout.resize(cpls.size());
        for (size_t i = 0; i < drawLayout.size(); i++)
            drawLayout[i] = i;
    }

    lua_pop(lua,1);

    /*
        Transform feedback
    */
    std::vector<std::string> feedbackVaryings;

    lua_pushstring(lua, "feedbackVaryings");
    lua_gettable(lua, 1);

    if (lua_istable(lua, -1))
    {

        iterateLuaArray(lua, [&]()
        {
            const char* s = lua_tostring(lua, -1);
            if (s)
                feedbackVaryings.push_back(std::string(s));
        });
    }

    lua_pop(lua,1);

    {
        Log << "New configured particle system '" << name << "'" << Trace(CHash("ParticleSystemInfo"));

        auto playout = [&](const ConfigurableParticleLayout& l)
        {
            Log << "	Binding " << l.binding << Trace(CHash("ParticleSystemInfo"));
            Log << "	Size " << l.size << Trace(CHash("ParticleSystemInfo"));
        };
        Log << "Update layout" << Trace(CHash("ParticleSystemInfo"));
        size_t index = 0;
        for (auto& i : cpls)
        {
            Log <<  "Attrib " << index << ":" << Trace(CHash("ParticleSystemInfo"));
            index++;
            playout(i);
        }
        Log << "Draw layout" << Trace(CHash("ParticleSystemInfo"));
        index = 0;
        for (auto& num : drawLayout)
        {
            Log << "Attrib " << index << ":" << Trace(CHash("ParticleSystemInfo"));
            index++;
            if (num >= 0)
                playout(cpls[num]);
        }

        Log << "Feedback varyings" << Trace(CHash("ParticleSystemInfo"));
        for (auto& s : feedbackVaryings)
        {
            Log << "	" << s << Trace(CHash("ParticleSystemInfo"));
        }
    }


    ConfigurableParticleSystemParameters cpsp;

    cpsp.layout = cpls;
    cpsp.drawLayout = drawLayout;


    std::string ushaderName = "system_" + name + "_update";
    std::string dshaderName = "system_" + name + "_draw";

    Shader* ushader = g->getAssets()->shaders.addElement(Hash(ushaderName));
    ushader
    ->addFragShader(AssetsFolder+"/shaders/"+updateShader[0])
    ->addVertShader(AssetsFolder+"/shaders/"+updateShader[1])
    ->setTransformFeedbackVaryings(feedbackVaryings);

    if (updateShader[2].size() > 0)
        ushader->addGeomShader(AssetsFolder+"/shaders/"+updateShader[2]);


    //Load the new shader
    g->getAssets()->shaders.init();

    auto particleSystem = new ConfigurableParticleSystem();

    particleSystem->configure(g, ushader, nullptr, cpsp);

    ParticleSystemParameters psp;
    psp.name = name;

    g->registerParticleSystem(particleSystem, &psp);




     

    return LuaFunctionWrapper::ReturnHandler<ConfigurableParticleSystem*>::get(lua, particleSystem);
}

/*

Lua table:

    Psystem.Layer #scenegraph layer
    Psystem.DrawType #additive, alpha, etc.
    Psystem.UpdateShader = [vs, gs, fs]
    Psystem.DrawShader = [vs, gs, fs]
    
    Psystem.ParticleLayout = [ParticleStructData, ParticleStructData ...] # as defined below

    Psystem.DrawLayout = [ParticleStructData, ParticleStructData ...] # as defined below
    Psystem.PreDraw #predraw callback, can adjust OpenGL state (set uniforms etc.)
    Psystem.NewParticleHandler #function takes in x parameters and returns a float array

ParticleStructData, essentially contains values passed in glVertexAttribPointer:

    Psdata.size 
    Psdata.binding = VELOCITY | POSITION | RANDOM | CONSTANT
    Psdata.constantValue
    Psdata.randomMin
    Psdata.randomMax
*/


void addConfigurableParticleSystemDrawer(Graphics* g, ConfigurableParticleSystem* cps, Shader* s, int layer)
{
    if (cps == nullptr || s == nullptr)
    {
        return;
    }


    auto* p = cps->addDrawer(layer);
    p->setShader(s);
}

void RegisterConfigurableParticleSystem(Graphics* g, lua_State* lua)
{
    lua_newtable(lua);

    #define l_def_integer(x, n) lua_pushinteger(lua, x); lua_setfield(lua, -2, n)

    l_def_integer(ConfigurableParticleLayout::Constant, "Constant");
    l_def_integer(ConfigurableParticleLayout::Position, "Position");

    l_def_integer(ConfigurableParticleLayout::Velocity, "Velocity");
    l_def_integer(ConfigurableParticleLayout::Random, "Random");

    lua_setglobal(lua, "ParticleLayout");

    #undef l_def_integer
}



luaL_Reg configurableParticleSystem_functions[] =
{
    {"Register", configurableParticleSystemRegister},
    {"AddDrawPass", LuaClosureWrap(addConfigurableParticleSystemDrawer, 1)},
    {0,0}
};
