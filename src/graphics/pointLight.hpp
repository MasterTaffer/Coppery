#pragma once
#include "game/engineDefs.hpp"
#include "color.hpp"
#include "drawable.hpp"


class ScissorBox
{
public:
    Vector2f top;
    Vector2f bottom;
};

class ScenePointLightLeaf;

/*! \brief Drawable for point lights
*/
class PointLight : public Drawable
{

    float power = 0.01;
    Color color;
    float height = 0.0;
    Vector2f pos = {0,0};
    bool ignoreFOV = false;
public:

    void setIgnoreFOV(bool i)
    {
        ignoreFOV = i;
    }

    bool getIgnoreFOV()
    {
        return ignoreFOV;
    }

    //! Set power of the light
    void setPower(float p);

    //! Set color of the light
    void setColor(Color c)
    {
        color = c;
    }

    //! Set height of the lgiht
    void setHeight(float h)
    {
        height = h;
    }

    //! Set position of the light
    void setPosition(DefVector2 p)
    {
        pos = p;
    }

    //! Get position of the light
    Vector2f getPosition()
    {
        return pos;
    }

    //! Get color of the light
    Color getColor()
    {
        return color;
    }

    //! Get power of the light
    float getPower()
    {
        return power;
    }

    //! Get height of the light
    float getHeight()
    {
        return height;
    }

    
    ScissorBox calculateScissorBox();

    void draw(Graphics*);

};
