#include "pointLight.hpp"
#include "scene.hpp"
#include "graphics.hpp"
#include "glState.hpp"
#include "scene.hpp"
#include "oGL.hpp"
#include "shader.hpp"

#include <cmath>

void PointLight::setPower( float p )
{
    power = p;
    if (power < 0)
        power = 0.01;
}


ScissorBox PointLight::calculateScissorBox()
{

    const float firstMultiplier = 1.f/8.f;
    const float secondMultiplier = 1.f/16.f;

    
    float det = sqrt(firstMultiplier*firstMultiplier+4*power*secondMultiplier);

    float sol = firstMultiplier+det;
    sol /= 2*secondMultiplier;


    //sol^2 = height^2 +r^2
    //sol^2 - height^2 = r^2

    sol = sqrt(sol*sol-height*height);

    if (sol < 0)
        sol = 0;

    ScissorBox sb;
    sb.top = pos-Vector2f(sol,sol);
    sb.bottom = pos+Vector2f(sol,sol);

    return sb;

}

void PointLight::draw(Graphics* graphics)
{


    if (getPower() <= 0)
        return;

    ScissorBox box = calculateScissorBox();

    ScissorBox oc = box;

    box.top -= graphics->getGLState()->camera;
    box.bottom -= graphics->getGLState()->camera;

    float ytemp = box.top.y;
    box.top.y = box.bottom.y;
    box.bottom.y = ytemp;

    box.top.y = graphics->getScaledDimensions().y-box.top.y;
    box.bottom.y = graphics->getScaledDimensions().y-box.bottom.y;

    auto dim =  graphics->getScaledDimensions();
    float bx, by;
    float tx, ty;

    bx = box.bottom.x / dim.x;
    by = box.bottom.y / dim.y;
    tx = box.top.x / dim.x;
    ty = box.top.y / dim.y;

    Color color = getColor();

    #ifndef COPPERY_HEADLESS
    glUniform3f(graphics->getGLState()->currentShader->uniforms.ulocLightColor,color.r,color.g,color.b);
    glUniform1f(graphics->getGLState()->currentShader->uniforms.ulocLightPower,getPower());
    glUniform3f(graphics->getGLState()->currentShader->uniforms.ulocLightPos,getPosition().x,getPosition().y,getHeight());
    #endif

    float mbx, mby;
    float mtx, mty;

    mbx = bx*2-1;
    mby = by*2-1;
    mtx = tx*2-1;
    mty = ty*2-1;

    #ifndef COPPERY_HEADLESS
    glBegin(GL_QUADS);

    glVertexAttrib2f(1,bx,ty);
    glVertexAttrib2f(2,oc.top.x,oc.top.y);
    glVertex3f(mbx,mty,0);

    glVertexAttrib2f(1,bx,by);
    glVertexAttrib2f(2,oc.top.x,oc.bottom.y);
    glVertex3f(mbx,mby,0);

    glVertexAttrib2f(1,tx,by);
    glVertexAttrib2f(2,oc.bottom.x,oc.bottom.y);
    glVertex3f(mtx,mby,0);

    glVertexAttrib2f(1,tx,ty);
    glVertexAttrib2f(2,oc.bottom.x,oc.top.y);
    glVertex3f(mtx,mty,0);

    glEnd();
    #endif

    return;
}
