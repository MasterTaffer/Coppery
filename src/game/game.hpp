#pragma once
#include <random>
#include <vector>
#include <string>
#include <chrono>

//! A class used to measure time accurately
class StopWatch
{
    std::chrono::high_resolution_clock::time_point timer;
public:
    //! Start the timer and reset the clock
    void start()
    {
        timer = std::chrono::high_resolution_clock::now();
    }
    
    //! Get time since starting the clock in microseconds
    long int since()
    {
        auto current = std::chrono::high_resolution_clock::now();
        auto span = std::chrono::duration_cast<std::chrono::microseconds>(current - timer);
        return span.count();
    }
    
    //! Get the time and restart the clock simultaneously
    long int sinceAndStart()
    {
        auto current = std::chrono::high_resolution_clock::now();
        auto span = std::chrono::duration_cast<std::chrono::microseconds>(current - timer);
        
        timer = current;
        return span.count();
    }
};



/*
    Engine: the glue that binds stuff together
*/

class ArgumentParser;
class Graphics;
class Window;
class SoundSystem;
class Organizer;
class BroadPhase;
class ScriptEngine;
class GameVariableManager;
class Control;

/*! \brief The organization of components that form the game engine
 * 
 * Initializes, updates and shuts down all the game logic components, such as
 * collision detection, script engine and the tile map handler. 
 *
 * Is also responsible of maintaining the TickRate of the game, and provides
 * utilities such as GetDeltaTime. Random number generation functionality also
 * resides within the Engine. 
*/
class Engine
{
    //The engine wide random generator
    std::default_random_engine randomizer;

    Graphics* graphics = nullptr;
    SoundSystem* soundSystem = nullptr;
    Window* window = nullptr;
    BroadPhase* broadPhase = nullptr;
    ScriptEngine* scriptEngine = nullptr;
    Control* control = nullptr;
    GameVariableManager* variableManager = nullptr;

    //! The current engine time measured in engine time units
    double time = 0.0f;
    
    //! The current step delta time
    double deltaTime = 1.0f;
    
    //! The "real" delta time, independent from TimeMultiplier settings
    double realDeltaTime = 1.0f;
    
    
    /*! \brief Boolean if the current engine tick is a main step
     * 
     * If deltaTime is less than 1.0, the step rate is higher than the defined
     * tick rate. In these cases it is useful to separate these intermediate
     * steps from the main step. The main step rate should exactly match the
     * engine tick rate.
     */
    bool isMainStep = false;
    
    
    double mainStepCounter = 0.0f;

    bool gameOn = false;
    bool gamePaused = false;
    bool shutGameDown = false;

    
    void loadNewMap(std::string);

    std::string mapToLoad;
    bool preparingToLoadMap;

    
    bool isFirstStep;
    StopWatch stopWatch;


public:

    SoundSystem* getSoundSystem(){return soundSystem;}
    Graphics* getGraphics(){return graphics;}
    Window* getWindow(){return window;}
    BroadPhase* getBroadPhase(){return broadPhase;}
    ScriptEngine* getScriptEngine() {return scriptEngine;}


    void printDebugInfo();

    void loadMap(std::string s);

    bool getIsMainStep() {return isMainStep;}

    void turnGameOff() {shutGameDown = true;}



    double getRealDeltaTime() {return realDeltaTime;};
    
    double getDeltaTime() {return deltaTime;};

    double getTime() {return time;};

    bool getGameOn() { return gameOn; }

    void setGameOn();
    
    void setGameOff();

    void setGamePaused(bool b) {gamePaused = b;};

    bool getGamePaused() {return gamePaused;}

    void init(ArgumentParser&, GameVariableManager* var, Graphics*,Window*,SoundSystem*,Control*);

    void deInit();

    void update();


    //! Returns a random integer
    int getRandomI() {return randomizer();};
    
    //! Returns a random integer that is positive
    int getRandomU() {return randomizer()&(0x7fffffff);};
    
    //Returns a random floating point number
    double getRandomFloat()
    {
        std::uniform_real_distribution<> dist(0, 1.0);
        return dist(randomizer);
    }


    //! Returns a random number between the minimum and maxmium
    int getRandomRange(int min, int max)
    {
        if (min == max)
            return min;
        int e = getRandomU();

        int sp = e%(max-min);
        return (min+sp);
    }

    
    void endCurrentMap();


    void restart();

    GameVariableManager* getVariableManager()
    {
        return variableManager;
    }
    
};
