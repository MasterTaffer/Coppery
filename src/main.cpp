/*! \file main.cpp
    \brief Contains the main loop and the startup routines
*/


#include "compiler.hpp"
#include "argumentParser.hpp"

#include "log.hpp"


#include <cassert>
#ifdef COPPERY_COMPILE_LIBRARY

#include <iostream>
#include <exception>

#include "control/crossFunctional.hpp"

#include "window/window.hpp"
#include "graphics/graphics.hpp"
#include "game/game.hpp"
#include "sound/sound.hpp"

#include "control/control.hpp"


#include "fileOperations.hpp"
#include "variable.hpp"


//! The function to start the engine in its full functionality
void gameMain(ArgumentParser& args)
{
    //this flag determines if the engine is about to do a full initialization
    bool doFullInitialization = true;
    int totalInitializations = 0;
    while (doFullInitialization)
    {
        doFullInitialization = false;
        bool exiting = false;
        Log << "Initializing virtual file system" << Trace(CHash("General"));

        VFS_Init(args);

        Log << "Allocating subsystems" << Trace(CHash("General"));
        Window* window = new Window();
        Graphics* graphics = new Graphics();
        Engine* engine = new Engine();
        SoundSystem* soundSystem = new SoundSystem();
        Control* control = new Control();



        try
        {
            totalInitializations += 1;

            Log << "Initializing variable manager" << Trace(CHash("General"));
            GameVariableManager* varManager = new GameVariableManager();

            //Create a few global variables

            //.FullRestart: min 0, max 1, default 0: set to do a complete program reinitialization
            
            varManager->makeIntegerLimits(".FullRestart", 0, 1, 0,
                std::function<bool(const GameVariable&)>([&](const GameVariable& var)
                {
                    doFullInitialization = (var.getInteger() != 0);
                    return true;
                }));


            //.Exit: min 0, max 1, default 0: set to exit main loop & program
            varManager->makeIntegerLimits(".Exit", 0, 1, 0,
                std::function<bool(const GameVariable&)>([&](const GameVariable& var)
                {
                    exiting = (var.getInteger() != 0);
                    return true;
                }));

            {
                //.TotalInitializations: counts the number of program reinitializations
                auto* gv = varManager->makeInteger(".TotalInitializations", totalInitializations);
                gv->makeConstant();
            }

            //.Cycles: counts the number of program main loop cycles
            auto* cyclesGameVar = varManager->makeInteger(".Cycles", 0);

            //Stop variable from polluting the log
            if (!args.getArgument("","--no-hide-cycles"))
                cyclesGameVar->hideFromTrace();


            Log << "Loading configuration files" << Trace(CHash("General"));

            {
                auto file = GetFileStream("user/config/graphics.cfg");
                varManager->getConfigCollection()->addFromFileToGroup(file.get(), "graphics");

                file = GetFileStream("user/config/sound.cfg");
                varManager->getConfigCollection()->addFromFileToGroup(file.get(), "sound");

                file = GetFileStream("static/config_default/graphics.cfg");
                varManager->getConfigCollection()->addFromFileToGroup(file.get(), "graphics");
                
                file = GetFileStream("static/config_default/sound.cfg");
                varManager->getConfigCollection()->addFromFileToGroup(file.get(), "sound");
            }

            //Activate and init systems
            Log << "Initializing window" << Trace(CHash("General"));
            window->init(varManager);

            Log << "Initializing graphics" << Trace(CHash("General"));
            graphics->init(varManager, window);

            Log << "Initializing sound system" << Trace(CHash("General"));
            soundSystem->init(varManager);

            Log << "Initializing control" << Trace(CHash("General"));
            control->init(args, varManager, window,graphics,engine,soundSystem);


            Log << "Initializing engine" << Trace(CHash("General"));
            engine->init(args, varManager, graphics,window,soundSystem, control);

            //Handle the --control_run command line flag
            {
                std::string runString;
                args.getArgumentFullString("", "--control-run", runString);
                if (runString.size() > 0)
                {
                    Log << "Executing control string" << Trace(CHash("General"));
                    control->runControlString(runString);
                }
            }

            Log << "Running poststart script" << Trace(CHash("General"));
            control->runScript("poststart.lua");

            Log << "Entering main loop" << Trace(CHash("General"));

            int loops = 0;
            while (!window->isClosing() && (doFullInitialization == false) && !exiting)
            {
                cyclesGameVar->setInteger(loops);

                
                //The following order of execution can be important
                //and should be carefully considered
                
                //Control controls, update it first
                control->update();
                
                //Run engine update after. Engine tick rate clock
                //might sleep for a moment here.
                engine->update();
                
                //Draw the new state. If something like v-sync is enabled,
                //a long wait might happen here
                graphics->update();
                
                //Poll the events after the possible waits in graphics && engine
                window->update();
                
                //Random sound stuff isn't that important
                soundSystem->update();

                loops += 1;
            }

            Log << "Exiting main loop after " << loops << " cycle(s)" << Trace(CHash("General"));

            //The order of deinitialization should be unspecified
            //But I don't believe that the systems are decoupled enough
            //for a clean exit to occur
            
            Log << "Deinitializing control" << Trace(CHash("General"));
            control->deInit();

            Log << "Deinitializing engine" << Trace(CHash("General"));
            engine->deInit();

            Log << "Deinitializing graphics" << Trace(CHash("General"));
            graphics->deInit();

            Log << "Deinitializing window" << Trace(CHash("General"));
            window->deInit();

            Log << "Deinitializing sound system" << Trace(CHash("General"));
            soundSystem->deInit();

            Log << "Deinitializing variable manager" << Trace(CHash("General"));
            delete varManager;
        }
        catch (const std::exception & e)
        {
            LogError << "Critical error: " << e.what() << Message(); 
        }

        Log << "Releasing subsystems" << Trace(CHash("General"));
        delete engine;
        delete window;
        delete graphics;
        delete soundSystem;
        delete control;

        Log << "Deinitializing virtual file system" << Trace(CHash("General"));
       
        VFS_Deinit();

        if (doFullInitialization)
        {
            Log << "Performing full reinitialization" << Trace(CHash("General"));
        }

    }
    Log << "Exiting" << Trace(CHash("General"));
}

#else //ifdef COPPERY_COMPILE_LIBRARY

#ifndef COPPERY_COMPILE_FMAIN
#define COPPERY_COMPILE_FMAIN
#endif

#endif

#ifdef COPPERY_COMPILE_FMAIN

void gameMain(ArgumentParser& args);

#ifdef __unix__

#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>

static void sigsegvHandler(int sig)
{
    const size_t btcount = 25;
    void *bt[btcount];
    size_t size;

    size = backtrace(bt, btcount);

    LogError << "FATAL ERROR: signal " << sig << Message();

    char** data = backtrace_symbols(bt, size);

    for (size_t i = 0; i < size; i++)
    {
        
        LogError << i << ": " ;
        if (data[i])
            LogError << data[i] << Message();
        else
            LogError << " unknown" << Message();
    }

    free(data);

    exit(1);
}
#endif // __unix__

//Included for sizeof checks
#include "vector2.hpp"
#include "game/engineDefs.hpp"


/*! \brief The main program
 * 
 * One probably shouldn't call this manually. Handles the construction of
 * ArgumentParser object and some common command line arguments. Calls
 * compilerMain or gameMain depending if the compiler only mode was set
 * by the command line flags. Also sets the correct Logger Trace values
 * for displaying somewhat sensible logging output on stdout.
 * 
 */

int main(int argc, const char** argv)
{
    
    argc--;
    argv++;



    ArgumentParser argumentParser(argc, argv);

    bool noCrashHandler = false;
    noCrashHandler = argumentParser.getArgument("", "--disable-crash-handler");


    bool compilerOnlyMode = false;  
    compilerOnlyMode = argumentParser.getArgument("-c", "--compiler");

    //All traces

    /*
        Trace(CHash("AngelScript"))
        Trace(CHash("AngelScriptError"))
        Trace(CHash("AngelScriptException"))
        Trace(CHash("AngelScriptLineCallback"))
        Trace(CHash("AngelScriptWarning"))
        Trace(CHash("ControllerInfo"))
        Trace(CHash("GameVarTrace"))
        Trace(CHash("General"))
        Trace(CHash("GraphicsWarning"))
        Trace(CHash("InputWarning"))
        Trace(CHash("MapInfo"))
        Trace(CHash("ParticleSystemInfo"))
        Trace(CHash("ShaderInfo"))
        Trace(CHash("Warning"))
    */


    bool verbose = argumentParser.getArgument("-v", "--verbose");
    bool veryVerbose = argumentParser.getArgument("-vv", "--very-verbose");
    bool veryVeryVerbose = argumentParser.getArgument("-vvv", "--very-very-verbose");

    if (veryVeryVerbose)
        veryVerbose = true;
    if (veryVerbose)
        verbose = true;

    if (!compilerOnlyMode)
    {

        Log.enableTrace(CHash("Warning"), "Warning: ");
        Log.enableTrace(CHash("GraphicsWarning"), "GraphicsWarning: ");
        Log.enableTrace(CHash("InputWarning"), "InputWarning: ");

        Log.enableTrace(CHash("AngelScriptError"), "AngelScriptError: ");
        Log.enableTrace(CHash("AngelScriptWarning"), "AngelScriptWarning: ");
        Log.enableTrace(CHash("AngelScriptException"), "AngelScriptException: ");
        Log.enableTrace(CHash("AngelScriptTesting"), "Tests: ");

        if (verbose)
        {
            Log.enableTrace(CHash("General"), "General: ");
            Log.enableTrace(CHash("AngelScript"), "AngelScript: "); 
        }
        
        if (veryVerbose)
        {
            Log.enableTrace(CHash("ParticleSystemInfo"), "ParticleSystemInfo: ");
            Log.enableTrace(CHash("MapInfo"), "MapInfo: ");
            Log.enableTrace(CHash("ShaderInfo"), "ShaderInfo: ");
            Log.enableTrace(CHash("ControllerInfo"), "ControllerInfo: ");
            Log.enableTrace(CHash("ControlLuaRequire"), "ControlLuaRequire: ");

            
        }
        if (veryVeryVerbose)
        {
            Log.enableTrace(CHash("GameVarTrace"), "GameVarTrace: ");
            Log.enableAllTraces(true);
        }
    }
    else
    {
        Log.enableTrace(CHash("Warning"), "Engine warning: ");

        
        if (verbose)
        {
            Log.enableTrace(CHash("AngelScript"), "Info: ");    
        }
        if (veryVerbose)
        {
            Log.enableTrace(CHash("General"), "General: ");    
        }
        if (veryVeryVerbose)
        {
            Log.enableAllTraces(true);
        }
        Log.enableTrace(CHash("AngelScriptError"),"");
        Log.enableTrace(CHash("AngelScriptWarning"),"");
    }

    Log << "Engine main startup" << Trace(CHash("General"));
    Log << VariableData(sizeof (size_t)) << Trace(CHash("General"));
    Log << VariableData(sizeof (DefVector2)) << Trace(CHash("General"));
    Log << VariableData(sizeof (Vector2d)) << Trace(CHash("General"));

    if (!noCrashHandler)
    {
        #ifdef __unix__
        Log << "Setting Error Handling procedures" << Trace(CHash("General"));
        signal(SIGSEGV, sigsegvHandler);
        
        struct rlimit core_limit = { RLIM_INFINITY, RLIM_INFINITY };
        auto b = (setrlimit( RLIMIT_CORE, &core_limit ) == 0);
        assert(b);
        (void)(b);
        #endif // __unix__
    }

    if (compilerOnlyMode)
    {   
        compilerMain(argumentParser);
    }
    else
    {
        gameMain(argumentParser);
    }
    

    return 0;
}


#endif 
