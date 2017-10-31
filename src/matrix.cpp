#include "matrix.hpp"


Matrix4 Matrix4::calculateOrtho(Vector2f dimensions, float zFar, float zNear)
{
    Matrix4 mat4;

    mat4.array[0][0]=2.0f/dimensions.x;
    mat4.array[1][0]=0;
    mat4.array[2][0]=0;
    mat4.array[3][0]=-1;

    mat4.array[0][1]=0;
    mat4.array[1][1]=-2.0f/dimensions.y;
    mat4.array[2][1]=0;
    mat4.array[3][1]=1;

    mat4.array[0][2]=0;
    mat4.array[1][2]=0;
    mat4.array[2][2]=2/(zFar-zNear);
    mat4.array[3][2]=(zNear+zFar)/(zNear-zFar);

    mat4.array[0][3]=0;
    mat4.array[1][3]=0;
    mat4.array[2][3]=0;
    mat4.array[3][3]=1;

    return mat4;
}

Matrix4 Matrix4::calculatePerspective(Vector2f dimensions, float zFar, float zNear, float fov)
{

    Matrix4 mat4;

    float ratio = dimensions.x/dimensions.y;

    float vert = 1.f/tan(0.5f * fov);

    mat4.array[0][0]=vert/ratio;
    mat4.array[1][0]=0;
    mat4.array[2][0]=0;
    mat4.array[3][0]=0;

    mat4.array[0][1]=0;
    mat4.array[1][1]=vert;
    mat4.array[2][1]=0;
    mat4.array[3][1]=0;

    mat4.array[0][2]=0;
    mat4.array[1][2]=0;
    mat4.array[2][2]=zFar/(zFar-zNear);
    mat4.array[3][2]=1;

    mat4.array[0][3]=0;
    mat4.array[1][3]=0;
    mat4.array[2][3]=-(zFar*zNear)/(zFar-zNear);
    mat4.array[3][3]=0;

    return mat4;
}
