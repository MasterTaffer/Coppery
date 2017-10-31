
#pragma once
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <array>
#include <list>

#include "vector2.hpp"
#include "hash.hpp"
#include "game/broadPhase.hpp"
#include "color.hpp"


//Thanks C++ for the forward declarin' fun
//TODO check all the delcarations

class Control;
class Engine;
class Sound;
class ScriptEngine;
class MapParameters;

class Tile;
class ScriptedActor;
class ScriptedProjectile;


//GHMAS classes
class ASContextPool;
class ASCoroutineStack;
namespace ASECS
{
    class EntitySystemManager;
}

//AngelScript classes

class asIScriptEngine;
class asIScriptObject;
class asIScriptModule;
struct asSMessageInfo;
class asITypeInfo;
class asIScriptContext;
class asIScriptFunction;

//AngelScript extensions

class CScriptDictionary;
class CScriptBuilder;

class ScriptDataTable;


//! Simple script function holder
class ScriptCallback
{
    asIScriptEngine* ase;
public:
    ScriptCallback(asIScriptEngine* e)
    {
        ase = e;
    }
    asIScriptFunction* callback = nullptr;
    void* object = nullptr;
    asITypeInfo* type = nullptr;
    void set(asIScriptFunction*);
    void release();
    int returnType;
};

//TODO toss this to a more sensible place
class ScriptTileCollisionCallback : public TileCollisionCallback
{
    asIScriptFunction* callback = nullptr;
    ScriptEngine* se;
public:
    ScriptTileCollisionCallback(ScriptEngine*);
    virtual ~ScriptTileCollisionCallback();
    void setCallback(asIScriptFunction*);
    virtual DefVector2 collide(PhysicsActor*, DefVector2, DefVector2) override;
};

class RefDrawableText;
class RefDrawableSprite;
class RefDrawableFillSprite;
class RefCharacterAnimator;
class RefPathfinder;
class RefDrawableStaticQuad;
class RefDrawableLine;
class RefSoundSource;
class RefPointLight;
class RefDrawableTilemap;
class UIWindow;


//! Get AngelScript type name
std::string GetTypeNameByTypeId(asIScriptEngine* ase, int typedid);


//TODO refactor BaseActor as PhysicsActor
//  Also figure out a better place for these
enum ScriptBaseActorFunctions
{
    ScriptBaseActorCollideActor = 0,
    ScriptBaseActorCollideStatic,
    ScriptBaseActorFunctionLast
};

enum ScriptBaseActorProperties
{
    ScriptBaseActorId = 0,

    ScriptBaseActorPosition,
    ScriptBaseActorSize,
    ScriptBaseActorCollisionOffset,

    ScriptBaseActorCollisionFlags,
    ScriptBaseActorCollisionType,
    ScriptBaseActorCollisionWith,

    ScriptBaseActorPropertyLast
};


//! RAII style Reentrancy checker
class CircularCallLockHolder
{
    bool denied;
    bool& lock;
public:
    /*! \brief Check & claim the reentrancy lock \p lock

        If the lock was already claimed, throw an AngelScript exception or a
        C++ exception depending if there is an active AngelScript context.
        The lockDenied() will also return true in this case.

        The \p lock is set to true when claimed and false when released.
        Therefore one should initialize the \p lock variable with false.
    */
    CircularCallLockHolder(bool& lock, const char* msg = nullptr);

    //! Destructor that releases the lock
    ~CircularCallLockHolder();

    //! Returns if the lock couldn't be acquired
    bool lockDenied();
};

class RequiredScriptType
{
public:
    std::string name;
    bool found = false;
    int typeId = 0;

    struct Property
    {
        std::string name;
        int type = 0;
        int index = 0;
        int offset = 0;
    };

    struct Function
    {
        std::string name;
        std::string declaration;
        asIScriptFunction* ptr = nullptr;
    };

    asITypeInfo* typeInfo = nullptr;

    std::vector<Property> properties;
    std::vector<Function> functions;

    const Property* getProperty(const char* name) const
    {
        for (auto& p : properties)
        {
            if (p.name == name)
                return &p;
        }
        return nullptr;
    }

    const Function* getFunction(const char* name) const
    {
        for (auto& p : functions)
        {
            if (p.name == name)
                return &p;
        }
        return nullptr;
    }

};


/*! \brief Script engine responsible for handling and running AngelScript

    Starts and runs the AngelScript scripts. 
*/
class ScriptEngine
{
    std::vector<asIScriptFunction*> postNewMapFunctions;
    std::vector<asIScriptFunction*> initFunctions;

    //Array of all tests
    std::vector<asIScriptFunction*> tests;


    std::list<ScriptCallback> scriptCallbackEndStep;


    std::vector<ScriptTileCollisionCallback*> tileCollisionCallbacks;

    Engine* engine = nullptr;
    asIScriptEngine* ase = nullptr;
    asIScriptModule* baseModule = nullptr;

    void messageCallback(const asSMessageInfo *);
    void exceptionCallback(asIScriptContext *ctx);
    void messageOut(std::string& s);

    void defineVector2();
    void defineGameVar();

    void defineCollisionFunctions();
    void defineGameFunctions();
    void defineDrawables();

    void definePath();
    void defineSound();
    void defineMap();
    void defineFile();

    void defineDataTable();
    void defineFormatString();

    bool lock_runCollisionDetection = false;
    void scrRunCollisionDetection();
    void scrClearCollisionDetection();

    DefVector2 scrGetFontCharacterSize(Hash::HashUInt);
    DefVector2 scrGetTextureSize(Hash::HashUInt);
    DefVector2 scrGetMousePosition();

    void scrSpawnParticle(Hash system, DefVector2 position, DefVector2 velocity, Color c, int count);
    void scrSetCamera(DefVector2);
    DefVector2 scrGetCamera();
    void scrSetDrawFOV(bool);

    void scrLineCollisionQuery(DefVector2 p1, DefVector2 p2, asIScriptFunction *cb);
    void scrBoxCollisionQuery(DefVector2 p1, DefVector2 p2, asIScriptFunction *cb);
    void scrCircleCollisionQuery(DefVector2,float, asIScriptFunction *cb);

    void scrRegisterMapCollisionCallback(int,asIScriptFunction* cb);
    void scrRegisterMapParamCallback(asIScriptFunction* cb);
    void scrRegisterMapSpawnObjectCallback(asIScriptFunction* cb);

    void scrAddEndStepCallback(asIScriptFunction *cb);
    bool scrGetTileIsBlocking(DefVector2);

    DefVector2 scrGetWindowDimensions();

    asIScriptFunction* mapParamCallback = nullptr;
    asIScriptFunction* mapSpawnObjectCallback = nullptr;

    RefDrawableText* factoryDrawableText();
    RefCharacterAnimator* factoryCharacterAnimator();
    RefDrawableSprite* factoryDrawableSprite();
    RefDrawableFillSprite* factoryDrawableFillSprite();
    RefPathfinder* factoryPathfinder();
    RefDrawableLine* factoryDrawableLine();
    RefPointLight* factoryPointLight();
    RefSoundSource* factorySoundSource(Sound*);
    RefDrawableStaticQuad* factoryDrawableStaticQuad();
    RefDrawableTilemap* factoryDrawableMapLayer();

    bool validateRST(RequiredScriptType& rst);

    bool initialized = false;

    ASCoroutineStack* coroutineStack = nullptr;
    ASContextPool* contextPool = nullptr;
    ASECS::EntitySystemManager* entitySystemManager = nullptr;

    
    asIScriptContext* mainContext;

    bool compilerOnly = false;
    size_t modulesBuilt = 0;

    std::vector<RequiredScriptType> requiredScriptTypes;

    bool initEngine();
    bool initModule(CScriptBuilder* builder, asIScriptModule* mod);
public:

    //! Adds a new required script type.
    void addRequiredScriptType(const RequiredScriptType&);

    //! Gets a reference to RST
    const RequiredScriptType* getRequiredScriptType(const char* name);


    //! Validate an script object type identity
    asIScriptObject* validateObject(void *ptr, int tid, asITypeInfo* ti);

    //! Returns if compiler only mode is active
    bool isCompilerOnly()
    {
        return compilerOnly;
    }
    
    //! Returns if the engine has been initialized
    bool successfulInit()
    {
        return initialized;
    }

    asIScriptFunction* getMapSpawnObjectCallback()
    {
        return mapSpawnObjectCallback;
    }


    //! Gets the primary script context
    asIScriptContext* getContext() {return mainContext;};

    //! Gets the pointer to AngleScript engine    
    asIScriptEngine* getScriptEngine() {return ase;};

    //! Gets the pointer to Engine
    Engine* getEngine() {return engine;}
    
    //! Write AS configuration to real \p file
    void writeEngineConfigToFile(const char* file);

    //! Run tests and output results to real \p output
    bool runTests(const std::string& output);

    //! Returns if any scripts have been built
    bool isBuilt()
    {
        return (baseModule != nullptr);
    }

    //! Initialize in compiler only mode
    void initCompilerOnly(Control* c);


    //! Loads a module from byte code, returns true on success
    bool loadModuleByteCode(const std::string& source, const std::string& name);

    //! Builds a module, returns true on success
    bool buildModule(const std::vector<std::string>& sourceFiles, const std::string& name);

    //! Builds a module and saves the resulting bytecode, returns true on success
    bool buildModuleSave(const std::vector<std::string>& sourceFiles, const std::string& name, const char* outFile);

    //! Initializes the script engine
    void init(Engine* e, Control* c);

    //! Builds and initializes the "base" module
    void initBaseModule();

    //! Steps the script engine, effectively calling all step callbacks
    void update();

    //! Calls the end step callbacks
    void endStep();

    //! Deinitializes the script engine
    void deInit();

    //! Calls the hardcoded new game handlers
    void newGame();

    //! Calls all registered initialization functions
    void callInitFunctions();


    //! Calls the OnPostNewMap functions
    void postNewMap();

    void getMapParameters(MapParameters*);

    //! Executes a AngelScript code string
    bool executeString(const char* str);

    /*! \brief Starts the coroutine stack
     * 
     * The context provided must be in prepared state
     */
    
    void runCoroutineStack(asIScriptContext* ctx);

    friend class PhysicsActorManager;
};
