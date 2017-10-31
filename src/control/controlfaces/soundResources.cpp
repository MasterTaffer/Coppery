
#include "sound/sound.hpp"
#include "soundResources.hpp"
#include "fileOperations.hpp"
#include "control/crossFunctional.hpp"

Sound* createSound(SoundSystem* ss, const char* name, const char* source)
{
    if (!name || !source)
        return nullptr;
    auto sound = ss->sounds.addElement(Hash(name));

    sound->setFilename(AssetsFolder+"/audio/"+source);
    return sound;
}

Sound* getSound(SoundSystem* ss, const char* str)
{
    if (str)
        return ss->sounds.getElement(Hash(str));
    return nullptr;
}

void loadSounds(SoundSystem* ss)
{
    ss->sounds.init();
}

luaL_Reg soundResources_functions[] =
{
    {"CreateSound", LuaClosureWrap(createSound, 1)},
    {"LoadSounds", LuaClosureWrap(loadSounds, 1)},
    {"GetSound", LuaClosureWrap(getSound, 1)},

    {0,0}
};


luaL_Reg soundFunctions_functions[] =
{
    {"SetDefaultDistance", LuaClassMemberWrap(Sound, setDefaultDistance)},
    {"SetIsMusic", LuaClassMemberWrap(Sound, setIsMusic)},
    {"SetIsLooped", LuaClassMemberWrap(Sound, setIsLooped)},
    {"SetDefaltVolume", LuaClassMemberWrap(Sound, setDefaultVolume)},
    {"SetPitchShift", LuaClassMemberWrap(Sound, setPitchShift)},
    {"SetAttenuation", LuaClassMemberWrap(Sound, setAttenuation)},
    {"Play", LuaClassMemberWrap(Sound, play)},
    {"PlayPositional", LuaClassMemberWrap(Sound, playPositional)},

    {0,0}
};
