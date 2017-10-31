#pragma once
#include <string>
#include "drawable.hpp"
#include "color.hpp"
#include "vertexBuffer.hpp"

class Font;
/*! \brief Drawable used for drawing text
*/
class DrawableText : public DrawableTranslatable
{
    std::string text;
    Font* font;
    bool dirty = true;
    bool centered = false;
    float scale = 1.0f;
    float width = -1.0f;
    VertexBuffer buffer;
    int bufferGlyphsSize = 0;
public:

    DrawableText();

    //! Set text centering
    void setCentering(bool b)
    {
        centered = b;
    }

    //! Set scale factor of the text
    void setScale(float s)
    {
        if (scale != s)
            dirty = true;
        scale = s;
    }

    //! Set text single line width
    void setWidth(float s)
    {
        width = s;
    }

    //! Set font
    void setFont(Font* f)
    {
        font = f;
    }

    //! Change the text
    void setText(const std::string& t)
    {
        if (text != t)
        {
            dirty = true;
            text = t;
        }
    }

    void draw(Graphics*);
};

/*! \brief Drawable for drawing a sprite that fills
*/
class DrawableFillSprite : public DrawableSprite
{
    float fillAmount;
    float fillAmountSecond;
    bool useSecondColor;
    Color fillColor;
    Color fillColorSecond;
public:

    
    void setUseTwoColors(bool b) {useSecondColor = b;};

    
    void setFillColor(Color firstColor, Color secondColor = Color());

    
    void setFillAmount(float fillFirst, float fillSecond = 0.0f);

    virtual void draw(Graphics*);
};
