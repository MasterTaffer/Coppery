#include "drawable.hpp"
#include "texture.hpp"
#include "oGL.hpp"
#include "glUtilities.hpp"
#include "graphics.hpp"
#include "graphicsData.hpp"
#include "glState.hpp"
#include "shader.hpp"
#include "scene.hpp"
#include <cassert>


Drawable::~Drawable()
{
    if (node != nullptr)
    {
        node->drawable = nullptr;
        node->setRemoval();
    }
    node = nullptr;
}

bool SceneDrawableLeaf::traverse()
{

    if (remove)
    {
        return true;
    }
    assert (drawable != nullptr);

    if (drawable->getIsHidden())
        return false;
    drawable->draw(graphics);
    return false;
}


void DrawableSprite::setTexture(Texture* t)
{
    tex =  t;
}

void DrawableSprite::draw(Graphics* g)
{
    if (tex == nullptr)
        return;
    Hash shaderHash = CHash("texturedShaded");
    Shader* shader = g->getAssets()->shaders.getElement(shaderHash);
    if (shader)
    {
        ShaderBinder shaderBinder(g, shader);
        TextureBinder texturer(g,tex);

        #ifndef COPPERY_HEADLESS
        glUniform4f(g->getGLState()->currentShader->uniforms.ulocFadeColor,color.r,color.g,color.b,alpha);
        #endif

        Vector2f rPos = pos;
        if (!cameraSpace)
            rPos += g->getTopLeft();
        if (rotation != 0.0f)
            Drawing::drawQuadRotated(g,rPos,tex->getDimensions()*scale,rotation,depth);
        else
            Drawing::drawQuad(g,rPos,tex->getDimensions()*scale,depth);
    }
}

void DrawableLine::setPoints(DefVector2 _p1, DefVector2 _p2)
{
    p1 = _p1;
    p2 = _p2;
}

void DrawableLine::draw(Graphics* g)
{
    Hash shaderHash = CHash("colored");
    Shader* shader = g->getAssets()->shaders.getElement(shaderHash);
    if (shader)
    {
        ShaderBinder shaderBinder(g, shader);
        Drawing::drawLine(g, p1, p2, color,depth,width);
    }
}

void DrawableQuad::draw(Graphics* g)
{

    Hash shaderHash = CHash("colored");
    Shader* shader = g->getAssets()->shaders.getElement(shaderHash);
    if (shader)
    {
        ShaderBinder shaderBinder(g, shader);
        Vector2f rPos = pos;
        if (!cameraSpace)
            rPos += g->getTopLeft();
        Drawing::drawQuadColored(g,rPos,size,fillColor,depth);
    }
}

void DrawableStaticQuad::draw(Graphics* g)
{
    Hash shaderHash = CHash("staticQuadTexturedColor");
    Shader* shader = g->getAssets()->shaders.getElement(shaderHash);
    if (shader)
    {
        ShaderBinder shaderBinder(g, shader);
        Vector2f rPos = pos;
        if (!cameraSpace)
            rPos += g->getTopLeft();

        TextureBinder texbinder(g, tex);

        #ifndef COPPERY_HEADLESS

        glUniform4f(shader->getUniform(CHash("FadeColor")),
            color.r, color.g, color.b, colorAlpha);

        int uid;
        uid = shader->getUniform(CHash("Position"));
        glUniform2f(uid, rPos.x , rPos.y);

        uid = shader->getUniform(CHash("Size"));
        glUniform2f(uid, size.x, size.y);

        uid = shader->getUniform(CHash("Depth"));
        glUniform1f(uid, depth);

        uid = shader->getUniform(CHash("MinUV"));
        glUniform2f(uid, uv1.x, uv1.y);

        uid = shader->getUniform(CHash("MaxUV"));
        glUniform2f(uid, uv2.x, uv2.y);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        #endif
    }
}

