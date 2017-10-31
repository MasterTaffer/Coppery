#include "characterAnimator.hpp"
#include "texture.hpp"
#include "glUtilities.hpp"
#include "log.hpp"
#include "graphics.hpp"
#include "glState.hpp"
#include "graphicsData.hpp"

#include "separatedFile.hpp"

#include "oGL.hpp"

#include <rapidxml.hpp>
#include <sstream>

CharacterAnimator::CharacterAnimator()
{

}
CharacterAnimator::~CharacterAnimator()
{

}



void CharacterAnimator::setFadeColor(Color c, float time)
{
    maxFadeTime = time;
    fadeTime = time;
    fadeColor = c;
}

void CharacterAnimator::setBgSpeed(float speed)
{
    bgFrameSpeedMul = speed;
}

void CharacterAnimator::setFgSpeed(float speed)
{
    fgFrameSpeedMul = speed;
}
void CharacterAnimator::clearBgFrame()
{
    if (currentAnimation == nullptr)
        currentFrame = 0;
}

void CharacterAnimator::clearAnimation()
{
    currentFrame = 0;
    currentSubFrame = 0;
    currentAnimation = 0;
}

void CharacterAnimator::clearFgAnimation()
{
    if (currentAnimation == nullptr)
        return;
    currentFrame = 0;
    currentSubFrame = 0;
    currentAnimation = 0;
}

void CharacterAnimator::playAnimation(Hash animation, int priority)
{
    if (currentAnimation && (currentAnimationPriority < priority))
        return;
    if (bgAnimation && (bgPriority < priority))
        return;
    
    if (set == nullptr)
        return;
    currentAnimationPriority = priority;
    fgFrameSpeedMul = 1.0;
    currentFrame = 0;
    currentSubFrame = 0;
    currentAnimation = set->getAnimation(animation);
    if (currentAnimation)
    if (currentAnimation->frames.size() == 0)
    {
        currentAnimation = 0;
    }
}

void CharacterAnimator::setAnimation(Hash animation, int priority)
{
    if (set == nullptr)
        return;
    bgPriority = priority;
    
    bgAnimation = set->getAnimation(animation);
    if (bgAnimation)
    if (bgAnimation->frames.size() == 0)
        bgAnimation = 0;
}

void CharacterAnimator::advance(float dt)
{

    if (set == nullptr)
        return;
    float fmul = 1.0;
    float fd = 1.0;

    if (currentAnimation)
    {
        fmul = fgFrameSpeedMul;
        if (currentFrame <= currentAnimation->frames.size())
            fd = currentAnimation->frames[currentFrame].delay;
    }
    else
    if (bgAnimation)
    {
        fmul = bgFrameSpeedMul;
        int fc = bgAnimation->frames.size();
        if (fc > 0)
            fd = bgAnimation->frames[currentFrame%fc].delay;
    }
    fd *= 5;

    currentSubFrame += dt*fmul;
    if (fd >= 0)
    if (currentSubFrame >= fd)
    {
        currentFrame++;
        currentSubFrame = 0;
    }

    fadeTime -= dt;

}


CharacterAnimationStep* CharacterAnimator::getCurrentStep()
{
    if (set == nullptr)
        return nullptr;

    if (currentAnimation)
    {
        if (currentFrame >= currentAnimation->frames.size())
        {
            currentAnimation = nullptr;
            currentFrame = 0;
        }
        else
            return &(currentAnimation->frames[currentFrame]);

    }
    if (currentAnimation == nullptr)
    {
        if (bgAnimation == nullptr)
            return nullptr;
        else
        {
            int i = currentFrame % bgAnimation->frames.size();
            return &(bgAnimation->frames[i]);
        }
    }
    return nullptr;
}

void CharacterAnimator::draw(Graphics * g)
{
    auto stp = getCurrentStep();
    if (stp == nullptr)
        return;
    unsigned int texI = stp->frame;

    if (set->sprites.size() <= texI)
        return;

    CharacterAnimationFrame caf = set->sprites[texI];
    Vector2f size;
    Texture* tex = caf.texture;

    size = tex->getDimensions();
    bool doMirroring = caf.isMirrored ^ mirrored;

    if (doMirroring)
        size.x *= -1;
    
    size *= scale;

    if (fadeTime > 0.0f)
    {
        ShaderBinder shaderBinder(g, g->getAssets()->shaders.getElement(CHash("texturedColor")));
        TextureBinder textureBinder(g, tex);

        #ifndef COPPERY_HEADLESS
        glUniform4f(g->getGLState()->currentShader->uniforms.ulocFadeColor, fadeColor.r, fadeColor.g, fadeColor.b, fadeTime/maxFadeTime);
        #endif

        Drawing::drawQuadRotated(g, pos, size, direction, depth);

    }
    else
    {
        ShaderBinder shaderBinder(g, g->getAssets()->shaders.getElement(CHash("texturedShaded")));
        TextureBinder textureBinder(g,tex);
        
        #ifndef COPPERY_HEADLESS
        glUniform4f(g->getGLState()->currentShader->uniforms.ulocFadeColor, 1.0, 1.0, 1.0, alpha);
        #endif
        
        Drawing::drawQuadRotated(g, pos, size, direction, depth);

    }
}

void CharacterAnimator::setDirection(float dir)
{
    direction = dir;
}

void CharacterAnimator::setAnimationSet(CharacterAnimationSet *s)
{
    set = s;
    currentFrame = 0;
}

int CharacterAnimator::getFrameIntData(Hash key)
{
    auto stp = getCurrentStep();
    if (stp == nullptr)
        return 0;
    auto it = stp->dataInt.find(key);
    if (it == stp->dataInt.end())
        return 0;
    return (*it).second;
}

int CharacterAnimator::getAnimationIntData(Hash key)
{
    auto* p = bgAnimation;
    if (currentAnimation)
        p = currentAnimation;
    
    if (p == nullptr)
        return 0;

    auto it = p->dataInt.find(key);
    if (it == p->dataInt.end())
        return 0;
    return (*it).second;
}

#include "fileOperations.hpp"


void CharacterAnimationSet::init(GraphicsData* g)
{
    
    TextureMap& texMap = g->textures;

    Texture* null = texMap.getNull();
    Texture* lastValid = null;

    
    //Hardcoded filename
    //TODO replace this with something more sensible
    std::string filename = animFolder+"animations.xml";

    char* fileText = GetFileContentsCopy(filename);
    if (!fileText)
    {
        Log << animFolder << "animations.xml doesn't exist" << Trace(CHash("Warning"));;
        return;
    }

    rapidxml::xml_document<> doc;
    try
    {
        doc.parse<0>(fileText);
    }
    catch (rapidxml::parse_error ex)
    {
        Log << ex.what() << Trace(CHash("Warning"));
    }
    std::unordered_map<std::string, int> animFrames;

    auto rootnode = doc.first_node("animations");
    int frameCount = 0;
    if (rootnode)
    {
        auto animationNode = rootnode->first_node("animation");
        while (animationNode)
        {
            auto att = animationNode->first_attribute("name");
            if (!att)
            {
                Log << "Skipping nameless animation definition at "<< animFolder <<  Trace(CHash("Warning"));
                continue;
            }

            std::string name = att->value();

            animations[Hash(name)] = CharacterAnimation();
            CharacterAnimation& ca = animations[Hash(name)];

            auto frameNode = animationNode->first_node("frame");
            //Go through the <frame> tags
            while (frameNode)
            {
                float delay = 1;
                auto delatt = frameNode->first_attribute("delay");
                if (delatt)
                {
                    std::stringstream ss;
                    ss << delatt->value();
                    ss >> delay;
                }
                std::string texname;
                auto srcatt = frameNode->first_attribute("src");
                if (!srcatt)
                {
                    //If frame has no "src" attribute it is invalid - skip it
                    continue;
                }
                texname = srcatt->value();


                auto it = animFrames.find(texname);
                int cf = 0;
                if (it == animFrames.end())
                {
                    animFrames[texname] = frameCount;

                    //Animation texture name differs from the real texture name

                    //Real texture name is animName+"."+texName

                    Texture* tex = texMap.tryGetElement(Hash(animName+"."+texname));
                    CharacterAnimationFrame caf;
                    if (tex == nullptr)
                    {
                        tex = lastValid;
                    }
                    lastValid = tex;

                    caf.texture = tex;
                    sprites.push_back(caf);

                    cf = frameCount;
                    frameCount++;
                }
                else
                {
                    //The texture is already specified, reuse it
                    cf = it->second;
                }
                CharacterAnimationStep stp;
                stp.delay = delay;
                stp.frame = cf;

                //Go through optional parameters
                auto data_i = frameNode->first_node("int");
                while (data_i)
                {
                    auto key = data_i->first_attribute("key");
                    auto value = data_i->first_attribute("value");
                    if (!key || !value)
                        continue;

                    std::stringstream ss;
                    ss << value->value();
                    int i;
                    ss >> i;

                    std::string t = key->value();
                    stp.dataInt[Hash(t)] = i;
                    data_i = data_i->next_sibling("int");
                }
                ca.frames.push_back(stp);
                frameNode = frameNode->next_sibling("frame");
            }

            auto iNode = animationNode->first_node("int");
            //Iterate through <int> tags
            while (iNode)
            {

                auto key = iNode->first_attribute("key");
                auto value = iNode->first_attribute("value");
                if (!key || !value)
                    continue;

                std::stringstream ss;
                ss << value->value();
                int i;
                ss >> i;

                std::string t = key->value();
                ca.dataInt[Hash(t)] = i;
                iNode = iNode->next_sibling("int");
            }

            animationNode = animationNode->next_sibling("animation");
        }
    }
    else
    {
        Log << animFolder << "animations.xml" << " doesn't contain animations node" << Trace(CHash("Warning"));
    }

    delete[] fileText;

}

void CharacterAnimationSet::deInit()
{
    sprites.clear();
}

void CharacterAnimationMap::initializeElement(CharacterAnimationSet* s)
{
    s->init(assets);
}

void CharacterAnimationMap::deInitializeElement(CharacterAnimationSet* s)
{
    s->deInit();
}
