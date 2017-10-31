#pragma once
#include "elementMapper.hpp"
#include "game/engineDefs.hpp"
#include <string>
#include <list>
#include <random>
#include <memory>

namespace sf
{
    class SoundBuffer;
    class Sound;
    class Music;
}

class FileHandleStream;
class SoundSystem;
class GameVariableManager;

/*! \brief Class defining a single sound or music

    This is used when playing all kinds of sounds or music. There
    is only one instance of the Sound class for all different sounds.

*/
class Sound
{

    float vol = 1.0f;
    float distance = 100.f;
    float attenuation = 600.f;
    float minPitch = 1.0f;
    float maxPitch = 1.0f;

    bool isMusic = false;
    

    bool looped = false;
    std::string filename;
    void updateSoundData(sf::Sound* snd, float volume);
    void cleanUp();
    
    
    #ifndef COPPERY_HEADLESS
    std::vector<std::unique_ptr<sf::Sound>> playingSounds;

    std::unique_ptr<sf::Music> music;
    std::unique_ptr<sf::SoundBuffer> source;
    std::unique_ptr<FileHandleStream> fileHandle;
    #endif

    SoundSystem* soundSystem;
public:

    //! Constructor
    Sound();
    
    //! Destructor
    ~Sound();

    //! Gets the internal sound buffer
    sf::SoundBuffer* getSource()
    {
    #ifndef COPPERY_HEADLESS
        return source.get();
    #else
        return nullptr;
    #endif
    }


    //! Sets the filename of the audio source
    Sound* setFilename(const std::string& f)
    {
        filename = f;
        return this;
    }

    /*! \brief Sets the minimum distance for the sound
     * 
     * If the sound source is closer than this, the volume of the sound 
     * stops increasing.
     */
    Sound* setDefaultDistance(float d)
    {
        distance = d;
        return this;
    }

    //! Set the attenuation factor of the sound.
    Sound* setAttenuation(float d)
    {
        attenuation = d;
        return this;
    }

    //! Sets if the sound is music
    Sound* setIsMusic(bool i = true)
    {
        isMusic = i;
        return this;
    }

    //! Sets if the sound should loop
    Sound* setIsLooped(bool l = true)
    {
        looped = l;
        return this;
    }

    //! Set the default volume 
    Sound* setDefaultVolume(float v);

    //! Set the random pitch factor
    Sound* setPitchShift(float min, float max)
    {
        minPitch = min;
        maxPitch = max;
        return this;
    }

    //! Play the sound using a position
    void playPositional(DefVector2 pos, float volume = 1.0f);

    //! Play the sound
    void play();

    //! Play the sound and get a handle
    sf::Sound* playForSource();

    //! Play the sound positionally and get a handle
    sf::Sound* playForSource3D(DefVector2);

    //! Initialize the sound
    void load(SoundSystem*);

    //! Deinitialize the sound
    void unload();
};

//! ElementMapper for Sound
class SoundMap : public ElementMapper<Sound>
{
    SoundSystem* soundSystem;
public:
    //! Set the SoundSystem used for managing sounds
    void setSoundSystem(SoundSystem* s)
    {
        soundSystem = s;
    }
protected:
    void initializeElement(Sound*);
    void deInitializeElement(Sound*);
};


//! A changing source of sound
class SoundSource
{
    sf::Sound* playingSound = nullptr;
    SoundSystem* soundSystem = nullptr;
    Sound* sound = nullptr;
public:

    //! Set the sound to use
    void setSound(Sound* s) {sound = s;}


    //! Start playing the sound with volume 0
    void play();

    //! Start playing the sound positionally with volume 0
    void play(DefVector2);

    //! Set the volume of the sound source
    void setVolume(float vol);

    //! Set the position and velocity of the sound source
    void setPosition(DefVector2 pos, DefVector2 vel = {0,0});

    //! Stop the sound source
    void stop();

    SoundSource(SoundSystem* s) : soundSystem(s) {};

    ~SoundSource();
};

//! Sound system that initializes SFML sound
class SoundSystem
{
    std::default_random_engine srandom;
    DefVector2 soundObserver;
    double pitchMultiplier = 1.0;

    float masterVolumeSetting = 1.0f;
    float musicVolume = 1.0f;
    float musicVolumeSetting = 0.6f;
    float ambientVolumeSetting = 0.7f;
    float sfxVolumeSetting = 1.0f;
    float sfxVolume = 1.0f;
    float observerZ = 0.0f;
public:

    // TODO remove this setter/getter waltz
    
    void setSFXVolumeSetting(float vol);
    void setMusicVolumeSetting(float vol);
    void setMasterVolumeSetting(float vol);

    float getSFXVolumeSetting() {return sfxVolumeSetting*sfxVolume;}
    float getMusicVolumeSetting() {return musicVolumeSetting;}
    float getMasterVolumeSetting() {return masterVolumeSetting;}

    
    float getMusicVolume() {return musicVolume*musicVolumeSetting;}
    float getSoundVolume() {return sfxVolumeSetting;}
    float getAmbientVolume() { return sfxVolumeSetting*ambientVolumeSetting; }

    //! Set global pitch multiplier
    void setPitchMultiplier(double p);

    //! Get global pitch multiplier
    double getPitchMultiplier() {return pitchMultiplier;}

    //! Get current sound observer position
    DefVector2 getSoundObserver() {return soundObserver;}

    //! Set current sound observer position
    void setSoundObserver(DefVector2 o) {soundObserver = o;}

    //! Get random number between 0.0 and 1.0
    float getSRandom()
    {
        std::uniform_real_distribution<> dist(0, 1);
        return dist(srandom);
    }

    //! Initialize system
    void init(GameVariableManager* var);

    //! Deinitialize system
    void deInit();

    //! Stop the currently playing music
    void stopMusic();

    //! Seek music to position
    void seekMusic(double seconds);

    //! Update sound system
    void update();

    //TODO fix this nonsense as well
    sf::Music* playingMusic; 
    float playingMusicVolume = 1.0f;
    SoundMap sounds;
};

