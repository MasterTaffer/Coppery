#pragma once
#include "vector2.hpp"

//! Four-by-four matrix, useful in OpenGL vertex pipeline.
class Matrix4
{
public:
    
    //! The floating point data of the matrix
    union
    {
        float array[4][4];
        float data[16];
    };
    
    //! Returns a float pointer to the data
    const float* getGLArray()
    {
        return (const float*) data;
    }

    //! Calculates an orthographic projection matrix
    static Matrix4 calculateOrtho(Vector2f dimensions, float zFar, float zNear);
    
    //! Calculates a perspective projection matrix
    static Matrix4 calculatePerspective(Vector2f dimension, float zFar, float zNear, float fov);

};
