
#pragma once

//! Structure for three component additive RGB color
class Color
{
public:
    //! Red component
    float r;
    //! Green component
    float g;
    //! Blue component
    float b;
    
    //! Default constructor
    Color(){};

    //! Constructor from RGB values
    Color(float rn, float gn, float bn) : r(rn), g(gn), b(bn) {};

    //! Multiplication with scalar
    Color operator*(float f) const
    {
        Color c;
        c.r = f*r;
        c.g = f*g;
        c.b = f*b;
        return c;
    }

    //! Division with scalar
    Color operator/(float f) const
    {
        Color c;
        c.r = r/f;
        c.g = g/f;
        c.b = b/f;
        return c;
    }
};
