#include <luawrap.hpp>
#include "variable.hpp"
#include "game/game.hpp"
#include "game/script/script.hpp"

#include "scriptEngineInterface.hpp"
#include "log.hpp"
#include "control/crossFunctional.hpp"

static bool buildModule(Engine* e, const std::vector<std::string>& sourceFiles, const std::string& name)
{
    ScriptEngine* se = e->getScriptEngine();
    if (se)
    {
        return se->buildModule(sourceFiles, name);
    }
    else
        Log << "Script engine BuildModule called when uninitialized" << Trace(CHash("Warning"));
    return false;
}

bool runTests(Engine* e, const std::string& outfile)
{
    ScriptEngine* se = e->getScriptEngine();
    if (se)
    {
        return se->runTests(outfile);
    }
    else
        Log << "Script engine RunTests called when uninitialized" << Trace(CHash("Warning"));
    return false;
}

static void callInitFunctions(Engine* e)
{
    ScriptEngine* se = e->getScriptEngine();
    if (se)
    {
        se->callInitFunctions();
    }
    else
        Log << "Script engine CallInitFunctions called when uninitialized" << Trace(CHash("Warning"));
}

static void writeEngineConfig(Engine* e, const char* file)
{
    ScriptEngine* se = e->getScriptEngine();
    if (se)
    {
        se->writeEngineConfigToFile(file);
    }
    else
        Log << "Script engine WriteEngineConfig called when uninitialized" << Trace(CHash("Warning"));
}



static bool executeString(Engine* e, const char* str)
{
    ScriptEngine* se = e->getScriptEngine();
    if (se)
    {
        return se->executeString(str);
    }
    else
        Log << "Script engine ExecuteString called when uninitialized" << Trace(CHash("Warning"));
    return false;
}



luaL_Reg scriptEngine_functions[] =
{
    {"BuildModule", LuaClosureWrap(buildModule, 1)},
    {"CallInitFunctions", LuaClosureWrap(callInitFunctions, 1)},
    {"RunTests", LuaClosureWrap(runTests, 1)},
    {"WriteEngineConfig", LuaClosureWrap(writeEngineConfig, 1)},
    {"ExecuteString", LuaClosureWrap(executeString, 1)},
    {0,0}
};
