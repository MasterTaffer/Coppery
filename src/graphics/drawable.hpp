#pragma once
#include "hash.hpp"
#include "vector2.hpp"
#include "game/engineDefs.hpp"
#include "color.hpp"
#include "reference.hpp"

class Graphics;
class Texture;

class SceneDrawableLeaf;

/*! \brief A thing that will be drawn
 * 
 * Everything tangible that should be drawn should be done through Drawable
 * instances. The sub-classes of Drawable can specialize the functionality
 * for different types of displaying.
 * 
 * Typically one calls Graphics::addDrawable(Drawable*) and
 * Graphics::deleteDrawable(Drawable*) to add or remove drawables from the
 * screen.
 * 
 * \ref draw is the actual method that performs the OpenGL calls required
 * for displaying the Drawable. It is automatically called by Graphics
 * after the Drawable instance is added to the scene.
 */
class Drawable
{
    MixinReferenceCounted;
    
    bool hidden = false;
protected:
    
    //! The scene node leaf associated with this Drawable
    SceneDrawableLeaf* node = nullptr;
    friend class SceneDrawableLeaf;
    friend class Graphics;
public:

    //! Sets if the Drawable is hidden
    void setIsHidden(bool isHidden) {hidden = isHidden;}

    //! Gets if the Drawable is hidden
    bool getIsHidden() {return hidden;}

    //! Draws the Drawable
    virtual void draw(Graphics*) = 0;

    
    //! Constructor
    Drawable(){}

    //! Virtual destructor
    virtual ~Drawable();

};

//! \brief Drawable that can be moved
class DrawableTranslatable : public Drawable
{
protected:
    //! Position of the drawable
    Vector2f pos = {0,0};
    
    //! Depth/Z-coordinate of the drawable
    float depth = 0.0f; 

    //! Is the Drawable drawn in camera space or directly to screen
    bool cameraSpace = true;
public:

    //! Setter for cameraSpace
    void setCameraSpace(bool cSpace)
    {
        cameraSpace = cSpace;
    }

    //! Setter for depth
    void setDepth(float d)
    {
        depth = d;
    }

    //! Getter for depth
    float getDepth()
    {
        return depth;
    }

    //! Setter for position
    void setPosition(DefVector2 p)
    {
        pos = p;
    }

    //! Getter for position
    Vector2f getPosition()
    {
        return pos;
    }

    //! Virtual destructor
    virtual ~DrawableTranslatable(){};
};

//! Drawable for a single sprite
class DrawableSprite : public DrawableTranslatable
{
    Texture* tex = nullptr;
    float rotation = 0.0f;
    Vector2f scale = {1,1};
    Color color = Color(1,1,1);
    float alpha = 1.0;
public:

    void setColor(Color c){color = c;}
    
    void setAlpha(float a){alpha = a;}

    void setScale(DefVector2 s){scale = s;}

    void setRotation(float r){rotation = r;}

    float getRotation(){return rotation;}

    void setTexture(Texture* t);

    virtual void draw(Graphics*);

    virtual ~DrawableSprite(){};
};

//! Drawable sprite with user specified UV coordinates
class DrawableStaticQuad : public DrawableTranslatable
{
    Vector2f uv1;
    Vector2f uv2;
    Vector2f size;
    Texture* tex = nullptr;
    Color color = Color(1,1,1);
    float colorAlpha = 0.0;
public:
    void setTexture(Texture* t){tex = t;}

    void setSize(DefVector2 s) {size = s;}

    void setColor(Color c, float alpha) { color = c; colorAlpha = alpha; }

    //! \brief Set sprite texture UV coordinates
    void setUV(DefVector2 u1, DefVector2 u2) {uv1 = u1; uv2 = u2;}

    virtual void draw(Graphics*);
};

class DrawableQuad : public DrawableTranslatable
{
    Color fillColor;
    Vector2f size;
public:

    void setSize(DefVector2 s) {size = s;}


    void setColor(Color color) {fillColor = color;}

    virtual void draw(Graphics*);
};



class DrawableLine : public Drawable
{
    Vector2f p1,p2;
    Color color;
    float depth = 0.0f;
    float width = 1.0f;
public:
    void setWidth(float w)
    {
        width = w;
    }

    void setDepth(float d)
    {
        depth = d;
    }

    float getDepth()
    {
        return depth;
    }

    void setColor(Color c) { color = c; }

    void setPoints(DefVector2,DefVector2);

    virtual void draw(Graphics*);

    virtual ~DrawableLine(){};

};
