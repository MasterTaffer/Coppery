#include "graphics/gfxFlags.hpp"
#include "graphics/graphics.hpp"

#include "window/window.hpp"

#include "game/game.hpp"

#include "game/broadPhase.hpp"
#include "game/script/script.hpp"

#include "variable.hpp"

#include "sound/sound.hpp"

#include "argumentParser.hpp"

#include <thread>
#include <chrono>
#include <cmath>

const char* EngineScriptsNotCompiledMessage = "Script engine reinitialization required";

void Engine::setGameOn()
{
    if (!scriptEngine->successfulInit())
    {
        Log << EngineScriptsNotCompiledMessage << Trace(CHash("Warning"));
        return;
    }
    gameOn = true;
    shutGameDown = false;
}

void Engine::setGameOff()
{
    if (!gameOn)
        return;

    if (!scriptEngine->successfulInit())
    {
        Log << EngineScriptsNotCompiledMessage << Trace(CHash("Warning"));
        return;
    }


    gameOn = false;
}

void Engine::loadNewMap(std::string map)
{
    if (!scriptEngine->successfulInit())
    {
        Log << EngineScriptsNotCompiledMessage << Trace(CHash("Warning"));
        return;
    }

    //graphics->setSpecularIntensity(0.4f);
    //graphics->setSpecularPower(8.1f);

    //graphics->getFlags()->drawFOV = true;

    graphics->clearParticles();


    scriptEngine->postNewMap();
}

void Engine::init(ArgumentParser& args, GameVariableManager* var, Graphics* g,Window* w,SoundSystem* s, Control*c)
{
    time = 0.0f;
    isFirstStep = true;

    gameOn = false;
    gamePaused = false;
    preparingToLoadMap = false;

    randomizer.seed(std::chrono::system_clock::now().time_since_epoch().count());

    graphics = g;
    window = w;
    soundSystem = s;
    control = c;
    variableManager = var;

    //Note that this initialization sequence must be the same as in restart()

    broadPhase = new BroadPhase(this);
    scriptEngine = new ScriptEngine();

    scriptEngine->init(this, control);
    scriptEngine->initBaseModule();


    //Broadphase is dependent on the ScriptEngine
    broadPhase->init();


}
void Engine::deInit()
{
    if (gameOn)
        setGameOff();
    
    broadPhase->deInit();


    scriptEngine->deInit();

    delete broadPhase;
    delete scriptEngine;


    graphics = nullptr;
    window = nullptr;
    soundSystem = nullptr;
    control = nullptr;
    variableManager = nullptr;

    broadPhase = nullptr;
    scriptEngine = nullptr;
}
void Engine::update()
{
    if (!isFirstStep)
    {
        long int timerSpinLockThreshold; //microseconds
        
        timerSpinLockThreshold = variableManager->getIntegerDefault(CHash("Engine.SpinlockThreshold"), 100);
        
        int tickrate;
        
        tickrate = variableManager->getIntegerDefault(CHash("Engine.Tickrate"), 60);
        
        bool timeLocked;
        bool forceTickRate;
        
        timeLocked = (variableManager->getIntegerDefault(CHash("Engine.ForceDeltaTimeToOne"), 0) != 0);
        forceTickRate = (variableManager->getIntegerDefault(CHash("Engine.ForceTickRate"), 0) != 0);

        
        double minFrameTime;
        if (forceTickRate)
        {
            minFrameTime = 1000.0 / double(tickrate);
        }
        else
        {
            minFrameTime = graphics->getMinFrameTime();
        }
        
        long int tickMicros = minFrameTime * 1000;
        
        //The following loop works as a spinlock for the timer
        long int timed, excess;
        while(true)
        {
            timed = stopWatch.since();
            excess = tickMicros - timed;
            if (excess <= 0)
                break;
            
            if (excess >= timerSpinLockThreshold)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(excess - timerSpinLockThreshold));
                continue;
            }
        }
        
        //Log << "Micros since: " << stopWatch.since() << Message();
        
        stopWatch.start();
        
        double difference = double(timed) / 1000000.0;
        
        //Possible if the game froze for extremely long times due to 
        //integer overflow or something
        if (difference < 0)
        {
            Log << "Negative frame pacer difference" << Trace(CHash("Warning"));
            difference = 0;
        }
            
        //Log << "mft: " << minFrameTime << ", " << forceTickRate << ", " << timeLocked << ", " << difference <<  Message();

        if (!timeLocked)
        {
            deltaTime = difference * tickrate;
            if (deltaTime > 1.0)
                deltaTime = 1.0;
        }
        else
        {
            deltaTime = 1.0;
        }
    }
    else
    {
        isFirstStep = false;
        deltaTime = 1.0;
        stopWatch.start();
    }
    
    realDeltaTime = deltaTime;

    double timeMultiplier = variableManager->getNumberDefault(CHash("Engine.TimeMultiplier"), 1.0);
    deltaTime *= timeMultiplier;
    if (gamePaused)
        deltaTime = 0.0;
    
    mainStepCounter += deltaTime;
    isMainStep = false;
    if (mainStepCounter > 1.0)
    {
        mainStepCounter -= 1.0;
        isMainStep = true;
    }

    graphics->setDeltaTime(deltaTime);

        
    if (gameOn && !gamePaused)
    {
        //graphics->setDeltaTime(1.0f);
        //deltaTime = 1.0f;

        if (preparingToLoadMap)
        {
            preparingToLoadMap = false;
        }

        time += getDeltaTime();


        scriptEngine->endStep();


        if (shutGameDown)
        {
            setGameOff();
            shutGameDown = false;
        }
    }
    else
    {
        soundSystem->setPitchMultiplier(1.0f);
    }

}
void Engine::printDebugInfo()
{
    Log << "Engine debug information" << Trace(CHash("General"));

    Log << "...blah blah" << Trace(CHash("General"));
  
}

void Engine::loadMap(std::string s)
{
    mapToLoad = s;
    preparingToLoadMap = true;
}


void Engine::endCurrentMap()
{
}


void Engine::restart()
{
    if (gameOn)
        setGameOff();
    
    broadPhase->deInit();
    scriptEngine->deInit();      


    //Must be the same sequence as in the end of init();
    scriptEngine->init(this,control);
    scriptEngine->initBaseModule();
    broadPhase->init();
    
}
