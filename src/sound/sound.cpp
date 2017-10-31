#ifndef COPPERY_HEADLESS
#include <SFML/Audio.hpp>
#endif

#include "sound.hpp"
#include "log.hpp"

#include "separatedFile.hpp"
#include "fileHandleStream.hpp"
#include "fileOperations.hpp"

#include "variable.hpp"

void SoundSystem::init(GameVariableManager* var)
{
    std::vector<GameVariable*> gvars;
    gvars.push_back(var->makeNumberLimits("Sound.MasterVolume", 0.0, 1.0, 1.0,
        std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            setMasterVolumeSetting(gv.getNumber());
            return true;
        })));

    gvars.push_back(var->makeNumberLimits("Sound.EffectVolume", 0.0, 1.0, 0.7,
        std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            setSFXVolumeSetting(gv.getNumber());
            return true;
        })));

    gvars.push_back(var->makeNumberLimits("Sound.MusicVolume", 0.0, 1.0, 0.5,
        std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            setMusicVolumeSetting(gv.getNumber());
            return true;
        })));

    var->loadFromConfig(gvars, "sound");

    sfxVolume = 1.0f;
    sounds.setSoundSystem(this);

    observerZ = 200;


    SeparatedFile soundFile = SeparatedFile::BatchLoad(AssetsFolder+"/def/","sound",':');
    for (auto sr : soundFile.rows)
    {
        int ik = sr.getInt(2,0);
        bool looped = (ik&1);
        bool music = (ik&2);

        sounds.addElement(Hash(sr.getString(0)))->setFilename(AssetsFolder+"/audio/"+sr.getString(1))
        ->setIsMusic(music)->setIsLooped(looped)
        ->setDefaultVolume(sr.getDouble(3,1.0))->setPitchShift(sr.getDouble(4,1.0),sr.getDouble(5,1.0))
        ->setDefaultDistance(sr.getDouble(6,200.0))
        ->setAttenuation(sr.getDouble(7,10.0));

    }

    #ifndef COPPERY_HEADLESS
    sf::Listener::setGlobalVolume(masterVolumeSetting*100.f);
    #endif

    sounds.init();
}

void SoundSystem::setMasterVolumeSetting(float vol)
{
    masterVolumeSetting = vol;
    #ifndef COPPERY_HEADLESS
    sf::Listener::setGlobalVolume(masterVolumeSetting*100.f);
    #endif
}

void SoundSystem::setSFXVolumeSetting(float vol)
{
    sfxVolumeSetting = vol;
}

void SoundSystem::setMusicVolumeSetting(float vol)
{
    musicVolumeSetting = vol;
    if (playingMusic)
    {
        #ifndef COPPERY_HEADLESS
        playingMusic->setVolume(getMusicVolume()*playingMusicVolume*100);
        #endif
    }
}

void SoundSystem::setPitchMultiplier(double p)
{
    pitchMultiplier = p;
    if (playingMusic)
    {
        #ifndef COPPERY_HEADLESS
        playingMusic->setPitch(p);
        #endif
    }
}

void SoundSystem::seekMusic(double seconds)
{
    if (playingMusic)
    {
        #ifndef COPPERY_HEADLESS
        playingMusic->setPlayingOffset(sf::seconds(seconds));
        #endif
    }
}
void SoundSystem::deInit()
{
    stopMusic();

    sounds.deInit();

}

void SoundSystem::update()
{

    DefVector2 p = getSoundObserver();

    #ifndef COPPERY_HEADLESS
    sf::Listener::setPosition(p.x,p.y,observerZ);
    sf::Listener::setDirection(0,0,-1);
    sf::Listener::setUpVector(0,1,0);
    #endif

}



void SoundSystem::stopMusic()
{
    if (playingMusic)
    {
        #ifndef COPPERY_HEADLESS
        playingMusic->stop();
        #endif
    }
    playingMusic = nullptr;
}

Sound::Sound()
{
}


Sound* Sound::setDefaultVolume(float v)
{
    vol = v;
    return this;
}

void Sound::updateSoundData(sf::Sound* snd, float vol2)
{
    #ifndef COPPERY_HEADLESS
    float volume = vol*vol2*100.0;
    if (music)
        volume *= soundSystem->getMusicVolume();
    else
        volume *= soundSystem->getSoundVolume();
    float pShift = minPitch+soundSystem->getSRandom()*(maxPitch-minPitch);

    pShift *= soundSystem->getPitchMultiplier();

    snd->setVolume(volume);
    snd->setMinDistance(distance);
    snd->setAttenuation(attenuation);
    snd->setPitch(pShift);
    snd->play();
    #endif
    /*snd->setIsPaused(false);*/

}

void Sound::play()
{
    #ifndef COPPERY_HEADLESS
    if (isMusic)
    {
        if (soundSystem->playingMusic != music.get())
        {
            soundSystem->stopMusic();
            soundSystem->playingMusic = music.get();
            soundSystem->playingMusicVolume = vol;
            music->setVolume(soundSystem->getMusicVolume()*vol*100);
            music->play();
            music->setLoop(looped);

        }
        return;
    }
    if (!source)
        return;
    
    
    auto snd = std::make_unique<sf::Sound>(*source.get());

    snd->setRelativeToListener(true);
    
    updateSoundData(snd.get(),1.0f);
    cleanUp();
    playingSounds.push_back(std::move(snd));
    #endif
}

void Sound::playPositional(DefVector2 pos, float volume)
{
    #ifndef COPPERY_HEADLESS
    if (isMusic)
        return;
    if (!source)
        return;

    
    auto snd = std::make_unique<sf::Sound>(*source.get());
    snd->setPosition(pos.x,pos.y,0);
    snd->setRelativeToListener(false);

    updateSoundData(snd.get(),volume);


    cleanUp();
    playingSounds.push_back(std::move(snd));
    #endif
}

void Sound::cleanUp()
{
    #ifndef COPPERY_HEADLESS
    
    while(playingSounds.size())
    {
        sf::Sound* x = playingSounds[0].get();
        if (x == nullptr || x->getStatus() == sf::Sound::Stopped)
        {
            playingSounds.erase(playingSounds.begin(),playingSounds.begin()+1);
            continue;
        }
        break;
    }
    
    #endif
}

sf::Sound* Sound::playForSource()
{
    #ifndef COPPERY_HEADLESS
    if (isMusic)
        return nullptr;
    if (!source)
        return nullptr;
    sf::Sound* snd = new sf::Sound(*source.get());
    if (looped)
        snd->setLoop(true);
    updateSoundData(snd,0.0f);

    return snd;
    #else
    return nullptr;
    #endif
}

sf::Sound* Sound::playForSource3D(DefVector2 pos)
{
    #ifndef COPPERY_HEADLESS
    if (isMusic)
        return nullptr;
    if (!source)
        return nullptr;
    sf::Sound* snd = new sf::Sound(*source.get());
    snd->setPosition(pos.x,pos.y,0);
    if (looped)
        snd->setLoop(true);
    
    updateSoundData(snd,0.0f);
    return snd;
    #else
    return nullptr;
    #endif
}


void Sound::load(SoundSystem* s)
{
    soundSystem = s;
 
    #ifndef COPPERY_HEADLESS
    source = nullptr;
    music = nullptr;

    if (isMusic)
    {
        auto p = GetFileStream(filename);
        if (p)
        {
            fileHandle = std::make_unique<FileHandleStream>(std::move(p));
            music = std::make_unique<sf::Music>();
            music->openFromStream(*(fileHandle.get()));
        }
        //music->openFromMemory(data,dlen);
    }
    else
    {
        const char* data;
        size_t dlen;

        data = GetFileContentsMaintainedCopy(filename, &dlen);
    
        if (data)
        {
            source = std::make_unique<sf::SoundBuffer>();
            source->loadFromMemory(data,dlen);
        }
    }
    #endif
}

Sound::~Sound()
{
}


void Sound::unload()
{
    #ifndef COPPERY_HEADLESS
    for (auto& p : playingSounds)
    {
        p->stop();
    }
    playingSounds.clear();
    
    music.reset();
    source.reset();
    #endif
}


void SoundMap::initializeElement(Sound* s)
{
    s->load(soundSystem);
}

void SoundMap::deInitializeElement(Sound* s)
{
    s->unload();
}



void SoundSource::play()
{
    #ifndef COPPERY_HEADLESS
    if (sound)
    {
        if (playingSound) stop();
        playingSound = sound->playForSource();
        setVolume(0.0f);
    }
    #endif
}

void SoundSource::play(DefVector2 pos)
{
    #ifndef COPPERY_HEADLESS
    if (sound)
    {
        if (playingSound) stop();
        playingSound = sound->playForSource3D(pos);
        setVolume(0.0f);
    }
    #endif
}


void SoundSource::setPosition(DefVector2 pos, DefVector2 vel)
{
    #ifndef COPPERY_HEADLESS
    if (playingSound)
    {
        playingSound->setPosition({static_cast<float>(pos.x),static_cast<float>(pos.y),0});
        //playingSound->setVelocity({vel.x,vel.y,0});
    }
    #endif
}



void SoundSource::setVolume(float vol)
{
    #ifndef COPPERY_HEADLESS
    if (playingSound)
    {
        playingSound->setVolume(soundSystem->getAmbientVolume()*vol);
    }
    #endif
}

void SoundSource::stop()
{
    #ifndef COPPERY_HEADLESS
    if (playingSound)
    {
        playingSound->stop();
        //playingSound->drop();
        delete playingSound;
        playingSound = nullptr;
    }
    #endif
}

SoundSource::~SoundSource()
{
    if (playingSound)
        stop();
}
