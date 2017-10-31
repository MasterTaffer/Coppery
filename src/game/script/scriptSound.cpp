
#include "script.hpp"
#include "sound/sound.hpp"
#include "game/game.hpp"
#include <angelscript.h>
#include <cassert>
#include "regHelper.hpp"

class RefSoundSource : public SoundSource
{
public:
    int ref;
    RefSoundSource(SoundSystem*s) : SoundSource(s)
    {
        ref = 1;
    }
    void addRef()
    {
        ref++;
    }

    void release()
    {
        ref--;
        if (ref <= 0)
        {
            delete this;
        }
    }

};

RefSoundSource* ScriptEngine::factorySoundSource(Sound* s)
{
    RefSoundSource* rs = new RefSoundSource(engine->getSoundSystem());
    rs->setSound(s);
    rs->play();
    return  rs;
}

void ScriptEngine::defineSound()
{
    SoundSystem* soundSystem = nullptr;
    SoundMap* soundMap = nullptr;
    if (!compilerOnly)
    {
         soundSystem = engine->getSoundSystem();
         soundMap = &soundSystem->sounds;
    }
  
    int r;
    r = ase->RegisterObjectType("Sound", 0, asOBJ_REF | asOBJ_NOCOUNT);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod("Sound", "void play()", asMETHODPR(Sound,play,(void),void), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod("Sound", "void play(Vector2, float vol = 1.0f)", asMETHODPR(Sound,playPositional,(DefVector2,float),void), asCALL_THISCALL);
    assert( r >= 0 );
    r = registerGlobalFunctionAux(this,"Sound@ GetSound(hash_t)", asMETHOD(SoundMap,getElement),asCALL_THISCALL_ASGLOBAL, soundMap);
    assert( r >= 0 );
    r = registerGlobalFunctionAux(this,"void StopMusic()", asMETHOD(SoundSystem,stopMusic),asCALL_THISCALL_ASGLOBAL, soundSystem);
    assert( r >= 0 );
    r = registerGlobalFunctionAux(this,"void SeekMusic(double)", asMETHOD(SoundSystem,seekMusic),asCALL_THISCALL_ASGLOBAL, soundSystem);
    assert( r >= 0 );

    
    r = registerGlobalFunctionAux(this,"void SetSoundPitchMultiplier(double)", asMETHOD(SoundSystem,setPitchMultiplier),asCALL_THISCALL_ASGLOBAL, soundSystem);
    assert( r >= 0 );
    
    
    r = ase->RegisterObjectType("SoundSource", 0, asOBJ_REF);
    assert( r >= 0 );

    ase->RegisterObjectBehaviour("SoundSource", asBEHAVE_FACTORY, "SoundSource@ f(Sound@)", asMETHOD(ScriptEngine,factorySoundSource), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );

    ase->RegisterObjectBehaviour("SoundSource", asBEHAVE_ADDREF, "void f()", asMETHOD(RefSoundSource,addRef), asCALL_THISCALL);
    assert( r >= 0 );

    ase->RegisterObjectBehaviour("SoundSource", asBEHAVE_RELEASE, "void f()", asMETHOD(RefSoundSource,release), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("SoundSource", "void play(Vector2)", asMETHODPR(RefSoundSource,play,(DefVector2),void), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("SoundSource", "void play()", asMETHODPR(RefSoundSource,play,(),void), asCALL_THISCALL);
    assert( r >= 0 );


    r = ase->RegisterObjectMethod("SoundSource", "void setVolume(float vol)", asMETHOD(RefSoundSource,setVolume), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("SoundSource", "void setPosition(Vector2,Vector2)", asMETHOD(RefSoundSource,setPosition), asCALL_THISCALL);
    assert( r >= 0 );

    (void)(r);
}
