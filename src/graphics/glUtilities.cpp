#include <cassert>
//for text printing
#include <cstring>

#include "glUtilities.hpp"
#include "font.hpp"
#include "graphics.hpp"
#include "graphicsData.hpp"

#include "glState.hpp"
#include "shader.hpp"
#include "gfxFlags.hpp"

#include "oGL.hpp"

TextureBinder::TextureBinder(Graphics* g,Hash key)
{
    Texture* t = g->getAssets()->textures.getElement(key);
    Bind(g, t);
}

TextureBinder::TextureBinder(Graphics* g,Texture* key)
{
    Bind(g, key);
}

void TextureBinder::Bind(Graphics* g, Texture* key)
{
    if (key == nullptr)
        return;


    #ifndef COPPERY_HEADLESS

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, key->getDiffuse());

    if (g->getFlags()->getIsNormalOn())
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, key->getNormal());
    }
    if (g->getFlags()->getIsSpecularOn())
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, key->getSpecular());
    }

    #endif

}

TextureBinder::~TextureBinder()
{
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D,0);
}

ShaderBinder::ShaderBinder(Graphics* g, Hash key)
{
    Shader* t = g->getAssets()->shaders.getElement(key);
    if (!t)
        return;


    active = true;
    state = g->getGLState();
    oldShader = state->currentShader;
    state->currentShader = t;


    #ifndef COPPERY_HEADLESS
    glUseProgram(t->getProgram());
    #endif

    state->updateGlobalUniforms();
}

ShaderBinder::ShaderBinder(Graphics* g, Shader* key)
{
    if (!key)
        return;

    active = true;
    state = g->getGLState();
    oldShader = state->currentShader;
    state->currentShader = key;

    #ifndef COPPERY_HEADLESS
    glUseProgram(key->getProgram());
    #endif

    state->updateGlobalUniforms();
}

ShaderBinder::~ShaderBinder()
{
    if (!active)
        return;
    
    state->currentShader = oldShader;


    #ifndef COPPERY_HEADLESS
    if (oldShader == nullptr)
        glUseProgram(0);
    else
        glUseProgram(oldShader->getProgram());
    #endif
}


ProjectionMatrixBinder::ProjectionMatrixBinder(Graphics* g, const Matrix4& m4)
{
    state = g->getGLState();
    oldMatrix = state->projection;
    state->projection = m4;

}

ProjectionMatrixBinder::~ProjectionMatrixBinder()
{
    state->projection = oldMatrix;
}


void Drawing::drawRectStatic(Graphics*,Texture* tex, Vector2f p1, Vector2f p2, float depth)
{
    assert(0);
    assert(tex != nullptr);
    Vector2f dim = tex->getDimensions();

    //TODO replace fixed pipeline stuff 

    #ifndef COPPERY_HEADLESS

    glBegin(GL_QUADS); 
    
    glVertexAttrib2f(1, p1.x/dim.x, p1.y/dim.y); 
    glVertex3f(p1.x,p1.y,depth); 

    glVertexAttrib2f(1,p1.x/dim.x,p2.y/dim.y); 
    glVertex3f(p1.x,p2.y,depth);

    glVertexAttrib2f(1,p2.x/dim.x,p2.y/dim.y);
    glVertex3f(p2.x,p2.y,depth);

    glVertexAttrib2f(1,p2.x/dim.x,p1.y/dim.y);
    glVertex3f(p2.x,p1.y,depth);

    glEnd(); 

    #endif
}

void Drawing::drawQuadColored(Graphics* g, Vector2f p, Vector2f size, Color color, float depth)
{
    assert(0);

    #ifndef COPPERY_HEADLESS
    glUniform4f(g->getGLState()->currentShader->uniforms.ulocFadeColor, color.r, color.g, color.b, 1.0f);

    glUniform3f(g->getGLState()->currentShader->uniforms.ulocDefaultNormal, 0.5, 0.5, 1);
    size /= 2;
    (void)g;
    float z = depth;
    glBegin(GL_QUADS);

    glVertex3f(p.x-size.x,p.y-size.y,z);

    glVertex3f(p.x-size.x,p.y+size.y,z);

    glVertex3f(p.x+size.x,p.y+size.y,z);

    glVertex3f(p.x+size.x,p.y-size.y,z);

    glEnd();
    #endif
}

void Drawing::drawQuad(Graphics* g, Vector2f p, Vector2f size, float depth)
{
    auto shader = g->getGLState()->currentShader;
    assert(shader != nullptr);


    #ifndef COPPERY_HEADLESS

    int uid;
    uid = shader->getUniform(CHash("Position"));
    glUniform2f(uid, p.x, p.y);

    uid = shader->getUniform(CHash("Size"));
    glUniform2f(uid, size.x, size.y);

    uid = shader->getUniform(CHash("Depth"));
    glUniform1f(uid, depth);

    uid = shader->getUniform(CHash("Rotation"));
    glUniform1f(uid, 0);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    #endif /* COPPERY_HEADLESS */
}


void Drawing::drawQuadRotated(Graphics* g, Vector2f pos, Vector2f size, float rotation, float depth)
{
    auto shader = g->getGLState()->currentShader;
    assert(shader != nullptr);


    #ifndef COPPERY_HEADLESS

    float rad = -rotation * 3.1415f / 180.f;

    int uid;
    uid = shader->getUniform(CHash("Position"));
    glUniform2f(uid, pos.x, pos.y);

    uid = shader->getUniform(CHash("Size"));
    glUniform2f(uid, size.x, size.y);

    uid = shader->getUniform(CHash("Depth"));
    glUniform1f(uid, depth);

    uid = shader->getUniform(CHash("Rotation"));
    glUniform1f(uid, rad);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    #endif /* COPPERY_HEADLESS */
}

void Drawing::drawLineQuad(Graphics* g, Vector2f p, Vector2f size, float depth)
{
    #ifndef COPPERY_HEADLESS
    size /= 2;
    (void)g;
    float z = depth;
    glBegin(GL_LINE_STRIP);

    glVertex3f(p.x-size.x,p.y-size.y,z);

    glVertex3f(p.x-size.x,p.y+size.y,z);

    glVertex3f(p.x+size.x,p.y+size.y,z);

    glVertex3f(p.x+size.x,p.y-size.y,z);

    glVertex3f(p.x-size.x,p.y-size.y,z);

    glEnd();

    #endif /* COPPERY_HEADLESS */
}

void Drawing::drawLine(Graphics* g, Vector2f p1, Vector2f p2, Color color, float depth, float width)
{
    Vector2f center = (p1 + p2) / 2;


    Vector2f dif = p2 - p1;
    float len = dif.length();
    if (len < 0.1f)
        return;
    
    Vector2f size;
    size.x = len;
    size.y = width;


    float rotation = dif.getAngleDegrees();

    
    //

    auto shader = g->getGLState()->currentShader;
    assert(shader != nullptr);
    #ifndef COPPERY_HEADLESS
    glUniform4f(shader->uniforms.ulocFadeColor, color.r, color.g, color.b, 1.0f);
    glUniform3f(shader->uniforms.ulocDefaultNormal, 0.5, 0.5, 1);

    #endif /* COPPERY_HEADLESS */

    drawQuadRotated(g, center, size, rotation, depth);
}

void Drawing::printText(Graphics* g,Font* f, const char* s,Vector2f pos,float scale, float width, bool centering,  float depth)
{
    assert(f != nullptr);

    #ifndef COPPERY_HEADLESS

    Vector2f dim = f->getCharacterDimensions() * scale;

    TextureBinder tex(g, f->getTexture());

    Shader* shader = g->getGLState()->currentShader;
    assert(shader != nullptr);

    float initX = pos.x;

    float xoff = 0.0f;
    if (centering)
    {
        for (const char* it = s; *it; ++it)
        {
            char c = *it;
            if (c == '\n')
                break;
            xoff -= dim.x / 2;
        }
    }
    pos.x += xoff;
  
    bool constrain = false;
    if (width >= dim.x)
        constrain = true;

    const char* ustr = s;
    std::string copy = "";

    size_t len = 0;
    if (constrain)
    {
        copy = s;
        
        float rowLeft = width;
        float rowSinceLastSpace = 0.0;
        int lastSpace = -1;
        for (int i = 0; size_t(i)< copy.size(); i++)
        {
            char c = copy[i];
            
            if (c == ' ')
            {
                lastSpace = i;
                rowSinceLastSpace = 0.0;
            }
            if (c == '\n')
            {
                lastSpace = -1;
                rowSinceLastSpace = 0.0;
                rowLeft = width;
            }
            else
            {
                if (rowLeft <= dim.x)
                {
                    if (lastSpace >= 0)
                    {
                        copy[lastSpace] = '\n';
                        lastSpace = -1;
                        rowLeft = width - rowSinceLastSpace;
                    }
                    else
                    {
                        copy.insert(i,"\n");
                        i++;
                        lastSpace = -1;
                        rowLeft = width;
                        rowSinceLastSpace = 0;
                    }
                    
                }

                rowLeft -= dim.x;
                if (c != ' ')
                  rowSinceLastSpace += dim.x;
            }
        }
        len = copy.size();
        ustr = copy.c_str();

    }
    else
    {
        if (centering)
            len = strlen(ustr);
    }

    
    //Fetch uniform locations
    
    int position_uid = shader->getUniform(CHash("Position"));
    int size_uid = shader->getUniform(CHash("Size"));
    int depth_uid = shader->getUniform(CHash("Depth"));
    int minuv_uid = shader->getUniform(CHash("MinUV"));
    int maxuv_uid = shader->getUniform(CHash("MaxUV"));
    
    //Depth is constant over the string
    //Other uniforms not so much
    glUniform1f(depth_uid, depth);

    int index = -1;  
    for (const char* it = s; *it; ++it)
    {
        char c = *it;
        index++;

        if (c == '\n')
        {
            if (centering)
            {
                xoff = 0.0f;
                for (size_t i = index+1; i < len; i++)
                {
                    char c2 = ustr[i];
                    if (c2 == '\n')
                        break;
                    xoff -= dim.x / 2;
                }
            }

            pos.x = initX+xoff;
            pos.y += dim.y;

            continue;
        }

        Vector2f uv1 = f->getUV(c);
        Vector2f uv2 = uv1+Vector2f(1.f/16.f,1.f/16.f);

        glUniform2f(position_uid, pos.x + dim.x / 2 , pos.y + dim.y / 2);

        glUniform2f(size_uid, dim.x, dim.y);

        glUniform2f(minuv_uid, uv1.x, uv1.y);

        glUniform2f(maxuv_uid, uv2.x, uv2.y);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        pos.x += dim.x;
    }
    
    #endif /* COPPERY_HEADLESS */
}

