#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include "dynamic.hpp"
#include "hash.hpp"

#define IS_MAIN_APPLICATION
#include "dynamicShared.hpp"
#include "crossFunctional.hpp"

/*
    The controller
    The master

    System that takes care of Lua scripting and extension module loading
*/
class Window;
class Graphics;
class Engine;
class UIMaster;
class SoundSystem;
class ArgumentParser;
class GameVariableManager;
struct lua_State;

//! Game engine controller
class Control
{
    DynamicManager dm;

    DynamicSharedInitializationData dsid;
    bool initialized = false;
    bool ownsLua = false;
    lua_State* lua;
    CrossFunction<void, const std::string&> logOutput;
    std::vector<std::string> logBuffer;

    //1: input hash, 2: layer, 3: callback
    std::vector<std::tuple<Hash, int, CrossFunction<void>>> luaInputHandlers;

    std::vector<CrossFunction<void>> luaUpdateHandlers;
public:

    //transfers the lua state
    //whoever gets it is responsible of closing the state AFTER control is closed
    
    //! Returns the lua_State and transfers ownership to caller
    lua_State* transferLua()
    {
        if (ownsLua)
        {
            ownsLua = false;    
            return lua;
        }
        
        throw std::logic_error("LUA ownership already transferred");
    }
    

    //! Extension module initialization function signature
    typedef bool(*ModuleInit)(MainApplication*, DynamicSharedInitializationData*);
    
    //! Pointer to Window subsystem
    Window* window;
    //! Pointer to Graphics subsystem
    Graphics* graphics;
    //! Pointer to Engine subsystem
    Engine* engine;
    //! Pointer to SoundSystem subsystem
    SoundSystem* soundSystem;
    
    //! List of all registered LuaInitHandler functions
    std::vector<LuaInitHandler> luaInitHandlers;
    //! List of all registered ASInitHandler functions
    std::vector<ASInitHandler> asInitHandlers;
    //! List of all registered InitHandler functions
    std::vector<InitHandler> initHandlers;
    //! List of all registered DeInitHandler functions
    std::vector<DeInitHandler> deInitHandlers;

    //! Call the \ref asInitHandlers with \p ase.
    void callAngelScriptInitHandlers(asIScriptEngine* ase);
    
    //! Add a new dynamic library
    bool addLibrary(const char* name);
    
    //! Add a new dynamic module
    void addModule(ModuleInit mi)
    {
        if (initialized)
            throw std::logic_error("Control modules cannot be added after initialization");
        modulesToLoad.push_back(mi);
    };
    
    //! Run a control command string
    void runControlString(const std::string&);
    
    //! Set a log callback function
    void setLogCallback(CrossFunction<void, const std::string&>);
    
    //! Initialize Control in compiler only mode
    void initCompilerOnly(ArgumentParser&);
    
    //! Initialize Control
    void init(ArgumentParser&,GameVariableManager* var, Window* w, Graphics* g, Engine* e, SoundSystem* s);
    
    //! Update and step Control
    void update();
    
    //! Deinitialize Control
    void deInit();
    
    //! Run a Lua script
    bool runScript(const std::string&);
    
    //! Add an input handler
    void addLuaInputHandler(const std::string&, int, CrossFunction<void>);
    
    //! Add an update handler
    void addLuaUpdateHandler(CrossFunction<void>);
    
    //! Return if the Control is in compiler only mode
    bool isCompilerOnly()
    {
        return compilerOnly;
    }
private:
    std::vector<ModuleInit> modulesToLoad;
    bool compilerOnly = false;
    int lastLuaTop = 0;
};


