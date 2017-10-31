#include "control.hpp"
#include "lua.hpp"
#include "luawrap.hpp"
#include "graphics/graphics.hpp"
#include "window/window.hpp"
#include "window/inputCascade.hpp"
#include "game/game.hpp"
#include "sound/sound.hpp"
#include "game/script/script.hpp"
#include "log.hpp"
#include "fileOperations.hpp"
#include "crossFunctional.hpp"
#include "argumentParser.hpp"

#include "control/controlfaces/configurableParticleSystemInterface.hpp"
#include "control/controlfaces/graphicsResources.hpp"
#include "control/controlfaces/configurableSceneNodeInterface.hpp"
#include "control/controlfaces/graphicsFunctions.hpp"
#include "control/controlfaces/gameVarInterface.hpp"
#include "control/controlfaces/scriptEngineInterface.hpp"
#include "control/controlfaces/soundResources.hpp"
#include "control/controlfaces/inputCascadeInterface.hpp"
#include "control/controlfaces/fileInterface.hpp"

template<typename T>
T* constructor()
{
    return new T;
}


luaL_Reg engine_functions[] =
{
    {"SetIsPaused", LuaClassMemberWrapStatic(Engine, setGamePaused)},
    {"On", LuaClassMemberWrapStatic(Engine, setGameOn)},
    {"Off", LuaClassMemberWrapStatic(Engine, setGameOff)},
    {"Restart", LuaClassMemberWrapStatic(Engine, restart)},
    {0,0}
};


luaL_Reg window_functions[] =
{
    {"Close", LuaClassMemberWrapStatic(Window, close)},
    {"GetDimensions", LuaClassMemberWrapStatic(Window,getWindowDimensions)},
    {"GetOutputDimensions", LuaClassMemberWrapStatic(Window,getOutputDimensions)},
    {"GetCharacterInput", LuaClassMemberWrapStatic(Window,getCharacterInput)},
    {"GetVideoModes", LuaClassMemberWrapStatic(Window,getVideoModes)},
    {"GetCurrentVideoModeIndex", LuaClassMemberWrapStatic(Window,getCurrentVideoModeIndex)},
    {"GetNewVideoModeIndex", LuaClassMemberWrapStatic(Window,getNewVideoModeIndex)},
    {"SetNewVideoModeIndex", LuaClassMemberWrapStatic(Window,setNewVideoModeIndex)},
    {"SetTitle", LuaClassMemberWrapStatic(Window,setTitle)},
    {"GetIsFullscreen", LuaClassMemberWrapStatic(Window,getIsFullScreen)},
    {"SetWindowIcon", LuaClassMemberWrapStatic(Window,setWindowIcon)},
    {0,0}
};


luaL_Reg graphics_functions[] =
{
    {"GetScreenSize", LuaClassMemberWrapStatic(Graphics, getScaledDimensions)},
    {0,0}
};


luaL_Reg sound_functions[] =
{
    {"GetMasterSoundVolume", LuaClassMemberWrapStatic(SoundSystem, getSFXVolumeSetting)},
    {"GetMasterMusicVolume", LuaClassMemberWrapStatic(SoundSystem, getMusicVolumeSetting)},
    {"GetMasterVolume", LuaClassMemberWrapStatic(SoundSystem, getMasterVolumeSetting)},

    {"SetMasterSoundVolume", LuaClassMemberWrapStatic(SoundSystem, setSFXVolumeSetting)},
    {"SetMasterMusicVolume", LuaClassMemberWrapStatic(SoundSystem, setMusicVolumeSetting)},
    {"SetMasterVolume", LuaClassMemberWrapStatic(SoundSystem, setMasterVolumeSetting)},

    {"MusicStop", LuaClassMemberWrapStatic(SoundSystem, stopMusic)},
    {0,0}
};


luaL_Reg control_functions[] =
{
    {"SetMessageCallback", LuaClassMemberWrapStatic(Control, setLogCallback)},
    {"AddInputHandler", LuaClassMemberWrapStatic(Control, addLuaInputHandler)},
    {"AddStepCallback", LuaClassMemberWrapStatic(Control, addLuaUpdateHandler)},
    {0,0}
};


void Lua_Log(const char* msg)
{
    Log << msg << Message();
}


void Control_Message(Control*, const char* msg)
{
    Log << msg << Message();
}

void Control_Register_ASHandler(Control* c, ASInitHandler asih)
{
    c->asInitHandlers.push_back(asih);
}

void Control_Register_LuaHandler(Control* c, LuaInitHandler luaih)
{
    c->luaInitHandlers.push_back(luaih);
}

void Control_Register_Init(Control* c, InitHandler ih)
{
    c->initHandlers.push_back(ih);
}

void Control_Register_DeInit(Control* c, DeInitHandler dih)
{
    c->deInitHandlers.push_back(dih);
}

bool Control::addLibrary(const char* name)
{
    auto lib = dm.loadLibrary(name);
    if (!lib)
    {
        Log << "Failed to load module " << name << Trace(CHash("Warning"));
        return false;
    }

    auto sym = dm.getSymbol<bool(*)(Control* , DynamicSharedInitializationData* )>(lib, "InitModule");
    if (!sym)
    {
        Log << "Failed to initialize module " << name << Trace(CHash("Warning"));
        return false;
    }
    Log << "Loaded control module " << name << Trace(CHash("General"));

    return sym(this, &dsid);
}

void Control::setLogCallback(CrossFunction<void, const std::string&> cb)
{
    logOutput = cb;
}


void Control::runControlString(const std::string& s)
{
    if (initialized == false || compilerOnly)
        throw std::logic_error("Control not initialized");

    //Log << s << Message();
    if (s.size() == 0)
        return;

    std::string sss = s;
    int top = lua_gettop(lua);
    bool err = luaL_dostring(lua,sss.c_str());
    (void)err;

    int ttop = lua_gettop(lua);
    if (ttop != top)
    {
        for (int i = top+1; i <= ttop; i++)
        {
            if (i != top+1)
                Log << ", ";
            if (lua_isstring(lua,i) || lua_isnumber(lua,i))
                Log << lua_tostring(lua,i);
            else if (lua_isboolean(lua,i))
                Log << (lua_toboolean(lua,i) ? "true" : "false");
            else if (lua_isnil(lua,i))
                Log << "nil";
            else 
                Log << "<" << lua_typename(lua,i) << "> " << lua_topointer(lua, i);

        }
        Log << Message();
    }
    lua_settop(lua,top);
    
}

void Control::update()
{

    if (logOutput)
    for (auto & s : logBuffer)
            logOutput(s);
    logBuffer.clear();




    for (auto& tpl : luaInputHandlers)
    {
        int layer = std::get<1>(tpl);
        if (window->getInputCascade()->getInput(std::get<0>(tpl), layer).getPressed())
            std::get<2>(tpl)();
    }

    for (auto& funsies : luaUpdateHandlers)
        funsies();

    int newTop = lua_gettop(lua);
    if (newTop != lastLuaTop)
    {
        Log << "Lua stack top position not balanced: " << lastLuaTop << " -> " << newTop << Trace(CHash("Warning"));
    }
    lastLuaTop = newTop;
}

void Control::addLuaInputHandler(const std::string& str, int layer, CrossFunction<void> fun)
{
    std::tuple<Hash, int, CrossFunction<void>> tpl(Hash(str), layer, fun);
    luaInputHandlers.push_back(tpl);
}

void Control::addLuaUpdateHandler(CrossFunction<void> fun)
{
    luaUpdateHandlers.push_back(fun);
}

void Control::initCompilerOnly(ArgumentParser& args)
{
    compilerOnly = true;
    init(args, nullptr, nullptr, nullptr, nullptr, nullptr);
}

void Control::init(ArgumentParser& args, GameVariableManager* var, Window* w, Graphics* g, Engine* e,  SoundSystem* s)
{
    window = w;
    graphics = g;
    engine = e;
    soundSystem = s;



    Log.setOutput([&](const std::string& op)
    {
        logBuffer.push_back(op);
    });




    dsid.version = DynamicSharedCurrentVersion;
    dsid.message = &Control_Message;
    dsid.registerASInitHandler = &Control_Register_ASHandler;
    dsid.registerLuaInitHandler = &Control_Register_LuaHandler;
    dsid.registerInitHandler = &Control_Register_Init;
    dsid.registerDeInitHandler = &Control_Register_DeInit;


    ownsLua = true;
    lua = luaL_newstate(); 
    luaL_openlibs(lua);



    LuaFunctionWrapper::FatPointer::createMetaTable(lua);

    //auto d = LuaFunctionWrapper::MethodWrapperGenerator<decltype(&Control::init), &Control::init>(&Control::init);
    
    //test(fsk::type());

    //Register functions for Lua

    luaL_newlibtable(lua, control_functions);
    lua_pushlightuserdata(lua, (void*) this);
    luaL_setfuncs(lua, control_functions,1);
    lua_setglobal(lua, "Control");


    auto fun = [](lua_State* lua) -> int
    {
        const char* name = luaL_checkstring(lua, 1);

        size_t size;
        Log << "Required " << name << Trace(CHash("ControlLuaRequire"));

        std::string copy = name;
        std::transform(copy.begin(), copy.end(), copy.begin(), [](char a){return a == '.' ? '/' : a; });

        char* s = GetFileContentsCopy(ControlScriptsFolder + "/" + copy + ".lua", &size);
        if (s)
        {
            luaL_loadbuffer(lua, s, size, copy.c_str());
            delete[] s;
            return 1;
        }
        return 0;
    };

    lua_getglobal(lua, "package");
    lua_getfield(lua, -1, "searchers");
    lua_pushinteger(lua, 1);
    lua_pushcfunction(lua, fun);
    lua_settable(lua, -3);
    lua_pop(lua, 2);

    
    lua_pushcfunction(lua, LuaCWrap(Lua_Log));
    lua_setglobal(lua, "print");


    lua_newtable(lua);

    for (auto& p : args.arguments)
    {
        lua_pushstring(lua, p.first.c_str());
        lua_pushstring(lua, p.second.c_str());
        lua_settable(lua, -3);
    }
    
    lua_setglobal(lua, "Args");





    //Register specialized functions

    if (!compilerOnly)
    {
        

        luaL_newlibtable(lua, window_functions);
        lua_pushlightuserdata(lua, (void*) window);
        luaL_setfuncs(lua,window_functions,1);
        lua_setglobal(lua, "Application");

        luaL_newlibtable(lua, gameVar_functions);
        lua_pushlightuserdata(lua, (void*) var);
        luaL_setfuncs(lua,gameVar_functions,1);
        lua_setglobal(lua, "GameVar");


        luaL_newlibtable(lua, engine_functions);
        lua_pushlightuserdata(lua, (void*) engine);
        luaL_setfuncs(lua,engine_functions,1);
        lua_setglobal(lua, "Game");

        lua_newtable(lua);
        //luaL_newlibtable(lua, sound_functions);
        lua_pushlightuserdata(lua, (void*) soundSystem);
        luaL_setfuncs(lua, sound_functions, 1);
        luaL_setfuncs(lua, soundFunctions_functions, 0);
        lua_setglobal(lua, "Sound");


        luaL_newlibtable(lua, scriptEngine_functions);
        lua_pushlightuserdata(lua, (void*) engine);
        luaL_setfuncs(lua, scriptEngine_functions, 1);
        lua_setglobal(lua, "ScriptEngine");


        luaL_newlibtable(lua, configurableParticleSystem_functions);
        lua_pushlightuserdata(lua, (void*) graphics);
        luaL_setfuncs(lua, configurableParticleSystem_functions, 1);
        lua_setglobal(lua, "ParticleSystem");


        lua_newtable(lua);
        //luaL_newlibtable(lua, graphicsResources_functions);
        lua_pushlightuserdata(lua, (void*) graphics);
        luaL_setfuncs(lua, graphicsResources_functions, 1);
        lua_pushlightuserdata(lua, (void*) soundSystem);
        luaL_setfuncs(lua, soundResources_functions, 1);
        lua_setglobal(lua, "Assets");

        luaL_newlibtable(lua, graphics_functions);
        lua_pushlightuserdata(lua, (void*) graphics);
        luaL_setfuncs(lua, graphics_functions, 1);
        lua_setglobal(lua, "Graphics");

        luaL_newlibtable(lua, graphicsFunctions_functions);
        lua_pushlightuserdata(lua, (void*) graphics);
        luaL_setfuncs(lua, graphicsFunctions_functions, 1);
        lua_setglobal(lua, "GraphicsDrawing");

        luaL_newlibtable(lua, configurableSceneNode_functions);
        lua_pushlightuserdata(lua, (void*) graphics);
        luaL_setfuncs(lua, configurableSceneNode_functions, 1);
        lua_setglobal(lua, "GraphicsNode");

        
        RegisterGraphicsFunctions(graphics, lua);
        RegisterConfigurableParticleSystem(graphics, lua);

        InputCascade* inputCascade = nullptr;
        if (window)
            inputCascade = window->getInputCascade();
        
        luaL_newlibtable(lua, inputCascade_functions);
        lua_pushlightuserdata(lua, (void*) inputCascade);
        luaL_setfuncs(lua, inputCascade_functions, 1);
        lua_setglobal(lua, "InputCascade");

        

        luaL_newlibtable(lua, file_functions);
        luaL_setfuncs(lua, file_functions, 0);
        lua_setglobal(lua, "File");

        RegisterFileFunctions(lua);
        
    }


    for (auto& i : modulesToLoad)
    {
        i(this, &dsid);  
    }

    runScript("modules.lua");


    if (!compilerOnly)
    {
        for (auto& i : initHandlers)
        {
            i();
        }

        for (auto& i : luaInitHandlers)
        {
            i(lua);
        }

        if (runScript("init.lua") == false)
        {
            Log << "Failed to run init.lua script" << Message();
        }
    }


    initialized = true;
    lastLuaTop = lua_gettop(lua);


}

void Control::callAngelScriptInitHandlers(asIScriptEngine* ase)
{
    for (auto& i : asInitHandlers)
    {
        i(ase);
    }
}

bool Control::runScript(const std::string& fname)
{
    size_t len;
    char* s = GetFileContentsCopy(ControlScriptsFolder + "/" + fname, &len);
    if (s)
    {

        int err = luaL_loadbuffer(lua, s, len, fname.c_str());
        delete[] s;
        if (err)
        {
            Log << lua_tostring(lua, -1) << Trace(CHash("Warning"));
            lua_pop(lua,1);
            return false;
        }

        err = lua_pcall(lua, 0, LUA_MULTRET, 0);
        if (err)
        {
            Log << lua_tostring(lua, -1) << Trace(CHash("Warning"));
            lua_pop(lua,1);
            return false;
        }
    }
    else
    {
        Log << "Failed to run control script " << fname << Trace(CHash("Warning"));
        return false;
    }
    return true;
}

void Control::deInit()
{

    for (auto& i : deInitHandlers)
    {
        i();
    }

    luaInputHandlers.clear();
    luaUpdateHandlers.clear();
    logOutput = CrossFunction<void, const std::string&>();
    logBuffer.clear();


    Log.setOutput(std::function<void(const std::string&)>());




    window = nullptr;
    graphics = nullptr;
    engine = nullptr;
    soundSystem = nullptr;

    luaInitHandlers.clear();
    asInitHandlers.clear();
    initHandlers.clear();
    deInitHandlers.clear();
    modulesToLoad.clear();


    if (ownsLua)
       lua_close(lua);
    
    initialized = false;
    compilerOnly = false;
}
