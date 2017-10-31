#pragma once


#include <string>
#include <list>
#include <unordered_map>
#include "color.hpp"
#include "elementMapper.hpp"
#include "drawable.hpp"

class DrawableSprite;
class Texture;
class GraphicsData;
class CharacterAnimationSet;


class CharacterAnimationStep
{
public:
    int frame;
    float delay;
    std::unordered_map<Hash, int> dataInt;
};

class CharacterAnimation
{
public:
    std::vector<CharacterAnimationStep> frames;
    std::unordered_map<Hash, int> dataInt;
};

/*! \brief Generic animator
*/

class CharacterAnimator : public DrawableTranslatable
{
    CharacterAnimationSet* set = nullptr;

    float direction = 0.0;
    float fgFrameSpeedMul = 1.0;
    float bgFrameSpeedMul = 1.0;
    float frameDelay = 12;
    float currentSubFrame = 0;
    unsigned int currentFrame = 0;
    int specialFrame = -1;
    int currentAnimationPriority = 0;
    int bgPriority = 10;
    float specialFrameHold = 0;

    std::list<std::pair<int,int>> specialFrameQueue;
    float maxFadeTime = 1.0f;
    float fadeTime = 0.0f;
    Color fadeColor;

    CharacterAnimation* currentAnimation = nullptr;
    CharacterAnimation* bgAnimation = nullptr;

    CharacterAnimationStep* getCurrentStep();

    bool mirrored = false;

    float alpha = 1.0; 
    Vector2f scale {1,1};

public:
    CharacterAnimator();
    virtual ~CharacterAnimator();

    //! Change color of the sprite momentarily
    void setFadeColor(Color c, float time);

    //! Change animation set
    void setAnimationSet(CharacterAnimationSet* set);

    //! Set rotation of the sprite
    void setDirection(float dir);

    //! Set mirroring of the sprite
    void setMirroring(bool mirroring)
    {
        mirrored = mirroring;
    }

    //! Set alpha of the sprite
    void setAlpha(float a){alpha = a;}

    //! Set scale of the sprite
    void setScale(DefVector2 s){scale = s;}

    void draw(Graphics*);

    //! Advance the animation
    void advance(float);

    //! Set the background animation speed
    void setBgSpeed(float);

    //! Set the foreground animation speed
    void setFgSpeed(float);

    //! Get integer data embedded into the current frame 
    int getFrameIntData(Hash key);

    //! Get integer data embedded into the animation
    int getAnimationIntData(Hash key);

    void clearBgFrame();

    void setAnimation(Hash animation, int priority = 10);

    void playAnimation(Hash animation, int priority = 0);

    void clearAnimation();

    void clearFgAnimation();

    int getPlayingAnimationFrame()
    {
        if (currentAnimation == nullptr)
            return -1;
        return currentFrame;
    }

};

class CharacterAnimationFrame
{
public:
    Texture* texture = nullptr;
    bool isMirrored = false;
};


/*! \brief Collection of animations used by CharacterAnimator
*/

class CharacterAnimationSet
{
    std::string animFolder;
    std::string animName;
    std::vector<CharacterAnimationFrame> sprites;
    std::unordered_map<Hash,CharacterAnimation> animations;
public:

    CharacterAnimation* getAnimation(Hash a)
    {
        auto it = animations.find(a);
        if (it == animations.end())
        {
            return nullptr;
        }
        return &(it->second);
    }


    CharacterAnimationSet* setAnimationFolder(std::string s) {animFolder = s;return this;};
    CharacterAnimationSet* setAnimationName(std::string s) {animName = s; return this;};


    
    void init(GraphicsData*);

    
    void deInit();

    friend class CharacterAnimator;
};

/*! \brief CharacterAnimatorSet collection
*/
class CharacterAnimationMap : public ElementMapper<CharacterAnimationSet>
{
    GraphicsData* assets = nullptr;
public:

    void setAssets(GraphicsData* g) {assets = g;};
protected:

    void initializeElement(CharacterAnimationSet*);
    void deInitializeElement(CharacterAnimationSet*);
};
