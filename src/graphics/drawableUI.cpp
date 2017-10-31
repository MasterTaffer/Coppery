#include "drawableUI.hpp"
#include "glUtilities.hpp"
#include "graphicsData.hpp"
#include "glState.hpp"
#include "graphics.hpp"
#include "shader.hpp"
#include "oGL.hpp"


DrawableText::DrawableText()
{
    depth = -100.0f;
    cameraSpace = false;

    #ifndef COPPERY_HEADLESS
    buffer.setUsage(GL_DYNAMIC_DRAW);
    buffer.setTarget(GL_ARRAY_BUFFER);
    #endif
}

void DrawableText::draw(Graphics* g)
{
    if (!font)
        return;

    Hash shaderHash = CHash("textured");
    Shader* shader = g->getAssets()->shaders.getElement(shaderHash);
    if (shader)
    {
        ShaderBinder shaderBinder(g, shader);
        Vector2f rPos = getPosition();
        if (!cameraSpace)
            rPos += g->getTopLeft();

        //Drawing::printText(g, font, text.c_str(), rPos,scale,width,centered,depth);

        constexpr const int floatsPerVertex = 4;

        
        if (dirty)
        {
            //Log << "Generating buffer for text " << text << "...";
            Font::TextParameters tp;
            tp.scale = scale;

            auto vec = font->generateText(text.c_str(), tp);
            buffer.load(vec.data(), vec.size());
            bufferGlyphsSize = vec.size() / floatsPerVertex;
            dirty = false;
            //Log << " loaded " << buffer.getSize() << " floats!" << Message();
        }

        if (buffer.getName())
        {
            //Log << "Rendering " << VariableData(buffer.getName()) << Message();
            TextureBinder tex(g, font->getTexture());

            #ifndef COPPERY_HEADLESS

            int uid = shader->getUniform(CHash("Position"));
            glUniform2f(uid, rPos.x, rPos.y);

            uid = shader->getUniform(CHash("Size"));
            glUniform2f(uid, 2.0, 2.0);

            uid = shader->getUniform(CHash("Depth"));
            glUniform1f(uid, depth);

            uid = shader->getUniform(CHash("Rotation"));
            glUniform1f(uid, 0.0);

            glBindBuffer(GL_ARRAY_BUFFER, buffer.getName());

            //Font::generate text returns data in this format:

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                sizeof(float) * 4, (void*)(sizeof(float) * 0));

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                sizeof(float) * 4, (void*)(sizeof(float) * 2));

            glDrawArrays(GL_TRIANGLES, 0, bufferGlyphsSize);
            #endif

            g->getGLState()->bindDefaultAttribs();
        }
        
    }
}

void DrawableFillSprite::setFillColor(Color color, Color second)
{
    fillColor = color;
    fillColorSecond = second;
}

void DrawableFillSprite::setFillAmount(float fill, float fill2)
{
    fillAmount = fill;
    fillAmountSecond = fill2;
}

void DrawableFillSprite::draw(Graphics* g)
{
    ShaderBinder shaderBinder(g, CHash("texturedVerticalColorFill"));

    #ifndef COPPERY_HEADLESS
    glUniform2f(g->getGLState()->currentShader->uniforms.ulocFillAmount,fillAmount,fillAmountSecond);
    glUniform3f(g->getGLState()->currentShader->uniforms.ulocFadeColor,fillColor.r,fillColor.g,fillColor.b);
    if (useSecondColor)
        glUniform3f(g->getGLState()->currentShader->uniforms.ulocLightColor,fillColorSecond.r,fillColorSecond.g,fillColorSecond.b);
    else
        glUniform3f(g->getGLState()->currentShader->uniforms.ulocLightColor,fillColor.r,fillColor.g,fillColor.b);
    #endif

    DrawableSprite::draw(g);
}
