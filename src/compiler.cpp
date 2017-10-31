#include "compiler.hpp"
#include "game/script/script.hpp"

#include "control/control.hpp"

#include "fileOperations.hpp"


void compilerMain(ArgumentParser& args)
{

    VFS_Init(args);

    Control* control = new Control();
    ScriptEngine* scriptEngine = new ScriptEngine();

    try
    {
        Log << "Running compiler only mode; no output enabled" << Trace(CHash("General"));
        control->initCompilerOnly(args);
        scriptEngine->initCompilerOnly(control);
        scriptEngine->initBaseModule();
        
        control->deInit();
        scriptEngine->deInit();

    }
    catch (const std::exception & e)
    {
        LogError << "Critical error: " << e.what() << Message();
    }

    VFS_Deinit();

    delete scriptEngine;
    delete control;
}
