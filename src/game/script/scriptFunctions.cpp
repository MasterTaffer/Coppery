#include "script.hpp"

#include "game/broadPhase.hpp"

#include "game/game.hpp"

#include "graphics/gfxFlags.hpp"
#include "window/inputCascade.hpp"
#include "window/window.hpp"
#include "graphics/graphics.hpp"
#include "graphics/particle.hpp"
#include "graphics/graphicsData.hpp"

#include "sound/sound.hpp"

#include "log.hpp"
#include "regHelper.hpp"

#include <scripthandle/scripthandle.h>
#include <angelscript.h>
#include <cassert>
#include <cmath>

void ScriptCallback::set(asIScriptFunction* cb)
{
    release();
    if (cb && cb->GetFuncType() == asFUNC_DELEGATE)
    {
        object = cb->GetDelegateObject();
        type = cb->GetDelegateObjectType();
        callback = cb->GetDelegateFunction();

        ase->AddRefScriptObject(object, type);
        callback->AddRef();

        cb->Release();
    }
    else
    {
        callback = cb;
    }

    returnType = callback->GetReturnTypeId();
}

void ScriptCallback::release()
{
    if (callback)
        callback->Release();
    callback = nullptr;
    if (object)
        ase->ReleaseScriptObject(object,type);
    object = nullptr;
    type = nullptr;
}

void ScriptEngine::scrAddEndStepCallback(asIScriptFunction *cb)
{
    scriptCallbackEndStep.push_back(ScriptCallback(ase));
    scriptCallbackEndStep.rbegin()->set(cb);
}

void ScriptEngine::messageOut(std::string& s)
{
    Log << s << Message();
}


void ScriptEngine::scrSpawnParticle(Hash system, DefVector2 position, DefVector2 velocity, Color color, int count)
{
    engine->getGraphics()->spawnParticles(system, position, velocity, color, count);
}


DefVector2 ScriptEngine::scrGetFontCharacterSize(Hash::HashUInt font)
{
    return engine->getGraphics()->getAssets()->fonts.getElement(Hash(font))->getCharacterDimensions();
}

asIScriptObject* ScriptEngine::validateObject(void *ptr, int tid, asITypeInfo* ti)
{
    if (!ti)
        return nullptr;

    asITypeInfo* ot = ase->GetTypeInfoById(tid);
    if ((tid & asTYPEID_OBJHANDLE) && (tid & asTYPEID_SCRIPTOBJECT) && (ti == ot))
    {
        asIScriptObject *obj = *reinterpret_cast<asIScriptObject**>(ptr);
        return obj;
    }
    return nullptr;
}

void ScriptEngine::scrSetCamera(DefVector2 pos)
{
    engine->getGraphics()->setCameraCenter(pos);
    engine->getSoundSystem()->setSoundObserver(pos);
}

DefVector2 ScriptEngine::scrGetCamera()
{
    return engine->getGraphics()->getCameraCenter();
}



DefVector2 ScriptEngine::scrGetWindowDimensions()
{
    return engine->getGraphics()->getScaledDimensions();
}

DefVector2 ScriptEngine::scrGetTextureSize(Hash::HashUInt h)
{
    auto* t = engine->getGraphics()->getAssets()->textures.getElement(Hash(h));
    if (t)
        return t->getDimensions();
    return DefVector2(0, 0);
}


DefVector2 ScriptEngine::scrGetMousePosition()
{
    return engine->getGraphics()->getCameraMousePosition();
}

void ScriptEngine::scrSetDrawFOV(bool b)
{
    engine->getGraphics()->getFlags()->drawFOV = b;
}


static double scr_clamprange(double val, double min, double max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

static double scr_clampabs(double val, double range)
{
    if (range <= 0)
        return 0;
    return scr_clamprange(val, -range, range);
}



void ScriptEngine::defineGameFunctions()
{
    int r = 0;



    Window* window = nullptr;
    Graphics* graphics = nullptr;
    InputCascade* inputCascade = nullptr;

    if (!compilerOnly)
    {
        window = engine->getWindow();
        graphics = engine->getGraphics();
        inputCascade = window->getInputCascade();
    }



    r = ase->RegisterFuncdef("bool IntervalCallback()");
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"void AddEndStepCallback(IntervalCallback @cb)", asMETHOD(ScriptEngine, scrAddEndStepCallback), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"void Print(const string &in)", asMETHOD(ScriptEngine, messageOut), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);



    r = ase->RegisterObjectType("Input", 0, asOBJ_REF | asOBJ_NOCOUNT);
    assert (r >= 0);
    

    r = ase->SetDefaultNamespace("Input");
    assert(r >= 0);
    
    r = registerGlobalFunctionAux(this,"Input@ GetInput(hash_t, int layer)", asMETHOD(InputCascade, getInput), asCALL_THISCALL_ASGLOBAL, inputCascade);
    assert (r >= 0);
    
    r = ase->SetDefaultNamespace("");
    assert(r >= 0);

    r = ase->RegisterObjectMethod("Input", "int getState()", asMETHOD(Input,getState), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Input", "int getPressed()", asMETHOD(Input,getPressed), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Input", "int getReleased()", asMETHOD(Input,getReleased), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Input", "float getValue()", asMETHOD(Input,getValue), asCALL_THISCALL);
    assert( r >= 0 );

    r = registerGlobalFunctionAux(this,"Vector2 GetMousePosition()",  asMETHOD(ScriptEngine,scrGetMousePosition), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);


    r = registerGlobalFunctionAux(this,"int GetRandom(int, int)", asMETHOD(Engine,getRandomRange), asCALL_THISCALL_ASGLOBAL, engine);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"double GetRandom()", asMETHOD(Engine,getRandomFloat), asCALL_THISCALL_ASGLOBAL, engine);
    assert (r >= 0);


    r = registerGlobalFunctionAux(this,"void SpawnParticle(hash_t, Vector2, Vector2, Color, int)", asMETHOD(ScriptEngine, scrSpawnParticle), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);

    r= registerGlobalFunctionAux(this,"bool GetIsMainStep()", asMETHOD(Engine,getIsMainStep),asCALL_THISCALL_ASGLOBAL,engine);
    assert (r >= 0);

    r= registerGlobalFunctionAux(this,"bool GetGamePaused()", asMETHOD(Engine,getGamePaused),asCALL_THISCALL_ASGLOBAL,engine);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"void SetCamera(Vector2)", asMETHOD(ScriptEngine, scrSetCamera), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"Vector2 GetCamera()", asMETHOD(ScriptEngine, scrGetCamera), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);

   

    r = registerGlobalFunctionAux(this,"void EndCurrentMap()", asMETHOD(Engine, endCurrentMap), asCALL_THISCALL_ASGLOBAL, engine);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"double GetTime()", asMETHOD(Engine, getTime), asCALL_THISCALL_ASGLOBAL, engine);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"double GetRealDeltaTime()", asMETHOD(Engine, getRealDeltaTime), asCALL_THISCALL_ASGLOBAL, engine);
    assert (r >= 0);
    
    r = registerGlobalFunctionAux(this,"double GetDeltaTime()", asMETHOD(Engine, getDeltaTime), asCALL_THISCALL_ASGLOBAL, engine);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"bool GetAnyKey()", asMETHOD(Window, getAnyKeyPressed), asCALL_THISCALL_ASGLOBAL, window);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"void TurnGameOff()", asMETHOD(Engine, turnGameOff), asCALL_THISCALL_ASGLOBAL, engine);
    assert (r >= 0);

   
    r = registerGlobalFunctionAux(this,"double GetRealTime()", asMETHOD(Window, getTime), asCALL_THISCALL_ASGLOBAL, window);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"Vector2 GetWindowDimensions()", asMETHOD(ScriptEngine, scrGetWindowDimensions), asCALL_THISCALL_ASGLOBAL, this);
    assert (r >= 0);


    r = registerGlobalFunctionAux(this,"Vector2 GetTextureDimensions(hash_t)", asMETHOD(ScriptEngine, scrGetTextureSize), asCALL_THISCALL_ASGLOBAL, this);
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"Vector2 GetFontCharacterDimensions(hash_t)", asMETHOD(ScriptEngine, scrGetFontCharacterSize), asCALL_THISCALL_ASGLOBAL, this);
    assert(r >= 0);


    /*
    r = registerGlobalFunctionAux(this,"float remainder(float,float)", asFUNCTION(remainderf), asCALL_CDECL);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"float copysign(float,float)", asFUNCTION(copysignf), asCALL_CDECL);
    assert (r >= 0);
    */

    
    r = registerGlobalFunctionAux(this,"double remainder(double,double)", asFUNCTIONPR(remainder,(double, double), double), asCALL_CDECL);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"double copySign(double,double)", asFUNCTIONPR(copysign, (double, double), double), asCALL_CDECL);
    assert (r >= 0);



    r = registerGlobalFunctionAux(this,"double clamp(double val, double min = 0.0, double max = 1.0)", asFUNCTION(scr_clamprange), asCALL_CDECL);
    assert (r >= 0);

    r = registerGlobalFunctionAux(this,"double clampAbsolute(double val, double range = 1.0)", asFUNCTION(scr_clampabs), asCALL_CDECL);
    assert (r >= 0);

    (void)(r);
}
