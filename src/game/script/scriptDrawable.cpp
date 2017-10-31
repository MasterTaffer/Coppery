#include "script.hpp"
#include "game/game.hpp"
#include <angelscript.h>
#include <cassert>

#include "graphics/drawableUI.hpp"
#include "graphics/pointLight.hpp"
#include "graphics/graphics.hpp"
#include "graphics/graphicsData.hpp"
#include "graphics/characterAnimator.hpp"
#include "graphics/drawableTilemap.hpp"
#include "graphics/uniformMap.hpp"


#include "regHelper.hpp"


#define MixinGraphicsRegister \
    void registerToGraphics(int layer)\
    {\
        graphics->addDrawable(this, layer);\
    }\
\
    void releaseFromGraphics()\
    {\
        graphics->deleteDrawable(this);\
    }

/*
    Bunch of wrappers for Drawables. These add new registerToGraphics and
    releaseFromGraphics functions.
*/


class RefDrawableLine : public DrawableLine
{
public:
    MixinGraphicsRegister
    
    Graphics* graphics;
    RefDrawableLine(Graphics* g)
    {
        graphics = g;
    }


};

class RefDrawableStaticQuad : public DrawableStaticQuad
{
public:
    MixinGraphicsRegister
    
    Graphics* graphics;
    RefDrawableStaticQuad(Graphics* g)
    {
        graphics = g;
    }

    void setTextureFromHash(Hash::HashUInt ui)
    {
        setTexture(graphics->getAssets()->textures.getElement(Hash(ui)));
    }
};


class RefPointLight : public PointLight
{
public:
    MixinGraphicsRegister
    
    Graphics* graphics;
    RefPointLight(Graphics* g)
    {
        graphics = g;
    }
};

class RefDrawableSprite : public DrawableSprite
{
public:
    MixinGraphicsRegister
    
    Graphics* graphics;
    RefDrawableSprite(Graphics* g)
    {
        graphics = g;
    }

    void setTextureFromHash(Hash::HashUInt ui)
    {
        setTexture(graphics->getAssets()->textures.getElement(Hash(ui)));
    }
};

class RefDrawableFillSprite : public DrawableFillSprite
{
public:
    MixinGraphicsRegister
    
    Graphics* graphics;
    RefDrawableFillSprite(Graphics* g)
    {
        graphics = g;
    }


    void setTextureFromHash(Hash::HashUInt ui)
    {
        setTexture(graphics->getAssets()->textures.getElement(Hash(ui)));
    }
};

class RefDrawableText : public DrawableText
{
public:
    MixinGraphicsRegister
    
    Graphics* graphics;
    RefDrawableText(Graphics* g)
    {
        graphics = g;
    }

    void setFontFromHash(Hash::HashUInt ui)
    {
        setFont(graphics->getAssets()->fonts.getElement(Hash(ui)));
    }
};

class RefCharacterAnimator : public CharacterAnimator
{
public:
    MixinGraphicsRegister
    
    Graphics* graphics;
    
    RefCharacterAnimator(Graphics* g)
    {
        graphics = g;
    }

    void setAnimationSetFromHash(Hash::HashUInt ui)
    {
        setAnimationSet(graphics->getAssets()->characterAnimations.getElement(Hash(ui)));
    }
};


class RefDrawableTilemap : public DrawableTilemap
{
   
public:
    MixinGraphicsRegister
    
    Graphics* graphics;
    
    RefDrawableTilemap(Graphics* g) : DrawableTilemap(g)
    {
        graphics = g;
    }

};


class RefUniformMap : public UniformMap
{
    MixinReferenceCounted;
public:   
    
};

/*
    "Generic method registerers"
*/

template <class T>
void regDrawable(asIScriptEngine* ase, const char* name)
{
    int r;
    r = ase->RegisterObjectMethod(name, "void setIsHidden(bool)", asMETHOD(T,setIsHidden), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod(name, "void register(int layer = DrawableLayerMesh)", asMETHOD(T,registerToGraphics), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod(name, "void release()", asMETHOD(T,releaseFromGraphics), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectBehaviour(name, asBEHAVE_ADDREF, "void f()", asMETHOD(T, addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour(name, asBEHAVE_RELEASE, "void f()", asMETHOD(T, release), asCALL_THISCALL);
    assert( r >= 0 );
    (void)(r);

}

template <class T>
void regDrawableSprite(asIScriptEngine* ase, const char* name)
{
    regDrawable<T>(ase,name);

    int r;

    r = ase->RegisterObjectMethod(name, "void setPosition(Vector2)", asMETHOD(T,setPosition), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setScale(Vector2)", asMETHOD(T,setScale), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setTexture(hash_t)", asMETHOD(T,setTextureFromHash), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setRotation(float)", asMETHOD(T,setRotation), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setDepth(float)", asMETHOD(T,setDepth), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setCameraSpace(bool)", asMETHOD(T,setCameraSpace), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "float getRotation()", asMETHOD(T,getRotation), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setColor(Color)", asMETHOD(T,setColor), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setAlpha(float)", asMETHOD(T,setAlpha), asCALL_THISCALL);
    assert( r >= 0 );
    (void)(r);
}

template <class T>
void regDrawableStaticQuad(asIScriptEngine* ase, const char* name)
{
    regDrawable<T>(ase,name);

    int r;
    r = ase->RegisterObjectMethod(name, "void setCameraSpace(bool)", asMETHOD(T,setCameraSpace), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setPosition(Vector2)", asMETHOD(T,setPosition), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setSize(Vector2)", asMETHOD(T,setSize), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setColor(Color, float)", asMETHOD(T,setColor), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setTexture(hash_t)", asMETHOD(T,setTextureFromHash), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setUV(Vector2, Vector2)", asMETHOD(T,setUV), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setDepth(float)", asMETHOD(T,setDepth), asCALL_THISCALL);
    assert( r >= 0 );
    (void)(r);
}


template <class T>
void regDrawableFillSprite(asIScriptEngine* ase, const char* name)
{
    regDrawableSprite<T>(ase,name);

    int r;

    r = ase->RegisterObjectMethod(name, "void setFillColor(Color,Color c = Color())", asMETHOD(T,setFillColor), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod(name, "void setUseTwoColors(bool)", asMETHOD(T,setUseTwoColors), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod(name, "void setFillAmount(float,float a = 0.0f)", asMETHOD(T,setFillAmount), asCALL_THISCALL);
    assert( r >= 0 );
    (void)(r);
}

template <class T>
void regDrawableLine(asIScriptEngine* ase, const char* name)
{
    regDrawable<T>(ase,name);

    int r;

    r = ase->RegisterObjectMethod(name, "void setPoints(Vector2, Vector2)", asMETHOD(T,setPoints), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setColor(Color)", asMETHOD(T,setColor), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setDepth(float)", asMETHODPR(T,setDepth,(float),void), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setWidth(float)", asMETHODPR(T, setWidth, (float), void), asCALL_THISCALL);
    assert(r >= 0);
    (void)(r);
}

template <class T>
void regDrawableTilemap(asIScriptEngine* ase, const char* name)
{
    regDrawable<T>(ase,name);

    int r;

    r = ase->RegisterObjectMethod(name, "void generate(Map::Layer&, Vector2, float)", asMETHOD(T,constructFromTilemap), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod(name, "void setUniforms(UniformMap&)", asMETHOD(T, setUniforms), asCALL_THISCALL);
    assert( r >= 0 );
    
    (void)(r);
}

template <class T>
void regCharacterAnimator(asIScriptEngine* ase, const char* name)
{
    regDrawable<T>(ase,name);

    int r;
    r = ase->RegisterObjectMethod(name, "void setPosition(Vector2)", asMETHOD(T,setPosition), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setAnimationSet(hash_t)", asMETHOD(T,setAnimationSetFromHash), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setBgSpeed(float)", asMETHOD(T,setBgSpeed), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setFgSpeed(float)", asMETHOD(T,setFgSpeed), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void advance(float)", asMETHOD(T,advance), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setDirection(float)", asMETHOD(T,setDirection), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setMirroring(bool)", asMETHOD(T,setMirroring), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setFadeColor(Color,float)", asMETHOD(T,setFadeColor), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setAnimation(hash_t,int priority = 10)", asMETHOD(T,setAnimation), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void playAnimation(hash_t,int priority = 0)", asMETHOD(T,playAnimation), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void clearAnimation()", asMETHOD(T,clearAnimation), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void clearFgAnimation()", asMETHOD(T,clearFgAnimation), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void clearBgFrame()", asMETHOD(T,clearBgFrame), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setDepth(float)", asMETHOD(T, setDepth), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod(name, "int getPlayingAnimationFrame()", asMETHOD(T,getPlayingAnimationFrame), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "int getPlayingFrameIntData(hash_t)", asMETHOD(T,getFrameIntData), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod(name, "int getAnimationIntData(hash_t)", asMETHOD(T,getAnimationIntData), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod(name, "void setScale(Vector2)", asMETHOD(T,setScale), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setAlpha(float)", asMETHOD(T,setAlpha), asCALL_THISCALL);
    assert( r >= 0 );
    
    (void)(r);
}

template <class T>
void regDrawableText(asIScriptEngine* ase, const char* name)
{
    regDrawable<T>(ase,name);

    int r;
    r = ase->RegisterObjectMethod(name, "void setCameraSpace(bool)", asMETHOD(T,setCameraSpace), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setCentering(bool)", asMETHOD(T, setCentering), asCALL_THISCALL);
    assert(r >= 0);
    r = ase->RegisterObjectMethod(name, "void setPosition(Vector2)", asMETHOD(T,setPosition), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setFont(hash_t)", asMETHOD(T,setFontFromHash), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setText(string &in)", asMETHOD(T,setText), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setScale(float)", asMETHOD(T,setScale), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setWidth(float)", asMETHOD(T,setWidth), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod(name, "void setDepth(float)", asMETHOD(T,setDepth), asCALL_THISCALL);
    assert( r >= 0 );
    (void)(r);
}

/*
    Constructors for the drawables.
*/

RefDrawableText* ScriptEngine::factoryDrawableText()
{
    return new RefDrawableText(engine->getGraphics());
}

RefCharacterAnimator* ScriptEngine::factoryCharacterAnimator()
{
    return new RefCharacterAnimator(engine->getGraphics());
}

RefDrawableSprite* ScriptEngine::factoryDrawableSprite()
{
    return new RefDrawableSprite(engine->getGraphics());
}

RefDrawableStaticQuad* ScriptEngine::factoryDrawableStaticQuad()
{
    return new RefDrawableStaticQuad(engine->getGraphics());
}

RefDrawableFillSprite* ScriptEngine::factoryDrawableFillSprite()
{
    return new RefDrawableFillSprite(engine->getGraphics());
}

RefDrawableLine* ScriptEngine::factoryDrawableLine()
{
    return new RefDrawableLine(engine->getGraphics());
}

RefPointLight* ScriptEngine::factoryPointLight()
{
    return new RefPointLight(engine->getGraphics());
}

RefDrawableTilemap* ScriptEngine::factoryDrawableMapLayer()
{
    return new RefDrawableTilemap(engine->getGraphics());
}


RefUniformMap* factoryUniformMap()
{
    return new RefUniformMap();
}


void ScriptEngine::defineDrawables()
{
    int r;
    
    
    r = ase->RegisterObjectType("UniformMap", 0, asOBJ_REF);
    assert( r >= 0 );
    
    r = ase->RegisterObjectBehaviour("UniformMap", asBEHAVE_ADDREF, "void f()", asMETHOD(RefUniformMap,addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("UniformMap", asBEHAVE_RELEASE, "void f()", asMETHOD(RefUniformMap,release), asCALL_THISCALL);
    assert( r >= 0 );

    
    r = ase->RegisterObjectBehaviour("UniformMap", asBEHAVE_FACTORY, "UniformMap@ f()", asFUNCTION(factoryUniformMap), asCALL_CDECL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("UniformMap", "void setUniform1f(hash_t, float)", asMETHOD(RefUniformMap, setUniform1f), asCALL_THISCALL);
    assert( r >= 0 );
    

    r = ase->RegisterEnum("DrawableLayer");
    assert(r >= 0);
    r = ase->RegisterEnumValue("DrawableLayer", "DrawableLayerMesh", SceneGraphOrder::SceneGraphMesh); assert(r >= 0);
    r = ase->RegisterEnumValue("DrawableLayer", "DrawableLayerLights", SceneGraphOrder::SceneGraphLights); assert(r >= 0);
    r = ase->RegisterEnumValue("DrawableLayer", "DrawableLayerLightlessMesh", SceneGraphOrder::SceneGraphLightlessMesh); assert(r >= 0);
    r = ase->RegisterEnumValue("DrawableLayer", "DrawableLayerAdditive", SceneGraphOrder::SceneGraphAdditive); assert(r >= 0);
    r = ase->RegisterEnumValue("DrawableLayer", "DrawableLayerAlpha", SceneGraphOrder::SceneGraphAlpha); assert(r >= 0);

    // CharacterAnimator

    r = ase->RegisterObjectType("CharacterAnimator", 0, asOBJ_REF);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("CharacterAnimator", asBEHAVE_FACTORY, "CharacterAnimator@ f()", asMETHOD(ScriptEngine,factoryCharacterAnimator), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );

    regCharacterAnimator<RefCharacterAnimator>(ase, "CharacterAnimator");


    // DrawableSprite

    r = ase->RegisterObjectType("Sprite", 0, asOBJ_REF);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("Sprite", asBEHAVE_FACTORY, "Sprite@ f()", asMETHOD(ScriptEngine,factoryDrawableSprite), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );


    regDrawableSprite<RefDrawableSprite>(ase, "Sprite");

    // DrawableFillSprite

    r = ase->RegisterObjectType("FillSprite", 0, asOBJ_REF);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("FillSprite", asBEHAVE_FACTORY, "FillSprite@ f()", asMETHOD(ScriptEngine,factoryDrawableFillSprite), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );

    regDrawableFillSprite<RefDrawableFillSprite>(ase, "FillSprite");

    // DrawableLine

    r = ase->RegisterObjectType("DrawLine", 0, asOBJ_REF);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("DrawLine", asBEHAVE_FACTORY, "DrawLine@ f()", asMETHOD(ScriptEngine,factoryDrawableLine), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );


    regDrawableLine<RefDrawableLine>(ase, "DrawLine");

    // DrawableText

    r = ase->RegisterObjectType("DrawText", 0, asOBJ_REF);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("DrawText", asBEHAVE_FACTORY, "DrawText@ f()", asMETHOD(ScriptEngine,factoryDrawableText), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );

    regDrawableText<RefDrawableText>(ase, "DrawText");

    // PointLight

    r = ase->RegisterObjectType("PointLight", 0, asOBJ_REF);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("PointLight", asBEHAVE_FACTORY, "PointLight@ f()", asMETHOD(ScriptEngine,factoryPointLight), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );

    //PointLight is registered by default to the Lights layer
    r = ase->RegisterObjectMethod("PointLight", "void setIsHidden(bool)", asMETHOD(RefPointLight,setIsHidden), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("PointLight", "void register(int layer = DrawableLayerLights)", asMETHOD(RefPointLight,registerToGraphics), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("PointLight", "void release()", asMETHOD(RefPointLight,releaseFromGraphics), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectBehaviour("PointLight", asBEHAVE_ADDREF, "void f()", asMETHOD(RefPointLight, addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("PointLight", asBEHAVE_RELEASE, "void f()", asMETHOD(RefPointLight, release), asCALL_THISCALL);
    assert( r >= 0 );


    r = ase->RegisterObjectMethod("PointLight", "void setPosition(Vector2)", asMETHOD(RefPointLight,setPosition), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod("PointLight", "void setPower(float)", asMETHOD(RefPointLight,setPower), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod("PointLight", "void setHeight(float)", asMETHOD(RefPointLight,setHeight), asCALL_THISCALL);
    assert( r >= 0 );
    r = ase->RegisterObjectMethod("PointLight", "void setColor(Color)", asMETHOD(RefPointLight,setColor), asCALL_THISCALL);
    assert( r >= 0 );



    // DrawableStaticQuad

    r = ase->RegisterObjectType("DrawQuad", 0, asOBJ_REF);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("DrawQuad", asBEHAVE_FACTORY, "DrawQuad@ f()", asMETHOD(ScriptEngine,factoryDrawableStaticQuad), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );


    regDrawableStaticQuad<RefDrawableStaticQuad>(ase, "DrawQuad");
    
    
    r = ase->RegisterObjectType("DrawableMapLayer", 0, asOBJ_REF);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("DrawableMapLayer", asBEHAVE_FACTORY, "DrawableMapLayer@ f()", asMETHOD(ScriptEngine,factoryDrawableMapLayer), asCALL_THISCALL_ASGLOBAL,this);
    assert( r >= 0 );

    regDrawableTilemap<RefDrawableTilemap>(ase, "DrawableMapLayer");

    
    

    Graphics* graphics = nullptr;
    if (!compilerOnly)
        graphics = engine->getGraphics();

    (void)(r);
 
}
