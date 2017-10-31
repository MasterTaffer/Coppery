#include "glState.hpp"
#include <cassert>

#include "shader.hpp"

#include "log.hpp"
#include "oGL.hpp"
#include "texture.hpp"
#include "gfxFlags.hpp"
#include <cmath>

GLState::GLState()
{
    nullShader = std::make_unique<Shader>();
    nullShader->initializedBasicUniforms = true;
    currentShader = nullShader.get();
}

GLState::~GLState()
{

}



void GLState::deInitializeFramebuffers()
{

    #ifndef COPPERY_HEADLESS

    if (scenePPFramebuffer)
    {
        glDeleteFramebuffers(1,&scenePPFramebuffer);
        glDeleteTextures(1,&scenePPTex);
    }

    
    if (sceneFOVFramebuffer)
    {
        glDeleteFramebuffers(1,&sceneFOVFramebuffer);
        glDeleteTextures(1,&sceneFOVTex);
    }

    #if GRAPHICS_USE_LIGHTING

    if (sceneFramebuffer)
    {
        glDeleteFramebuffers(1,&sceneFramebuffer);

        if (sceneNormalTex)
            glDeleteTextures(1,&sceneNormalTex);
        if (sceneSpecularTex)
            glDeleteTextures(1,&sceneSpecularTex);

        glDeleteTextures(1,&sceneDiffuseTex);
    }
    #endif

    if (sceneDepthTex)
        glDeleteTextures(1,&sceneDepthTex);


    if (quadBuffer)
        glDeleteBuffers(1, &quadBuffer);

    #endif

    sceneFramebufferTextureInterface.reset();
    sceneFramebufferDepthTextureInterface.reset();
    scenePPFramebufferTextureInterface.reset();
    
    sceneFramebuffer = 0;
    sceneFOVFramebuffer = 0;
    sceneDiffuseTex = 0;
    sceneNormalTex = 0;
    sceneSpecularTex = 0;
    sceneDepthTex = 0;
    sceneFOVTex = 0;
    scenePPFramebuffer = 0;
    scenePPTex = 0;
    quadBuffer = 0;
    
    framebuffers.clear();
}

void GLState::initializeFramebuffers(Vector2i dim, GFXFlags* settings)
{

    /*
        TODO: Refactor pls...
            Perhaps add a wrapper class for framebuffers and their textures,
            providing automatic deinitialization upon destruction.s

        The ugliness that is conditional framebuffer stuff
    */
    sceneFramebuffer = 0;
    sceneFOVFramebuffer = 0;
    sceneDiffuseTex = 0;
    sceneNormalTex = 0;
    sceneSpecularTex = 0;
    sceneDepthTex = 0;
    sceneFOVTex = 0;
    scenePPFramebuffer = 0;
    scenePPTex = 0;
    quadBuffer = 0;
    
    
    framebuffers.push_back(0);

    #ifndef COPPERY_HEADLESS

    std::vector<float> fdata;
    fdata.reserve(20);

    fdata.push_back(-1); fdata.push_back(1);
    fdata.push_back(0); fdata.push_back(1);

    fdata.push_back(-1); fdata.push_back(-1);
    fdata.push_back(0); fdata.push_back(0);

    fdata.push_back(1); fdata.push_back(1);
    fdata.push_back(1); fdata.push_back(1);
    
    fdata.push_back(1); fdata.push_back(-1);
    fdata.push_back(1); fdata.push_back(0);




    glGenBuffers(1, &quadBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
    glBufferData(GL_ARRAY_BUFFER, fdata.size() * sizeof(float), fdata.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);




    glGenFramebuffers(1,&scenePPFramebuffer);
    glGenTextures(1,&scenePPTex);
    
    framebuffers.push_back(scenePPFramebuffer);

    glBindTexture(GL_TEXTURE_2D, scenePPTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,dim.width,dim.height,0,GL_RGB,GL_FLOAT,0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    #if GRAPHICS_USE_LIGHTING

    glGenFramebuffers(1,&sceneFramebuffer);
    glGenTextures(1,&sceneDiffuseTex);
    
    framebuffers.push_back(sceneFramebuffer);

    glBindTexture(GL_TEXTURE_2D, sceneDiffuseTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,dim.width,dim.height,0,GL_RGB,GL_FLOAT,0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    if (settings->getIsNormalOn())
    {
        glGenTextures(1,&sceneNormalTex);
        glBindTexture(GL_TEXTURE_2D, sceneNormalTex);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,dim.width,dim.height,0,GL_RGB,GL_FLOAT,0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    if (settings->getIsSpecularOn())
    {
        glGenTextures(1,&sceneSpecularTex);
        glBindTexture(GL_TEXTURE_2D, sceneSpecularTex);
        glTexImage2D(GL_TEXTURE_2D,0,GL_R8,dim.width,dim.height,0,GL_RGB,GL_FLOAT,0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    #endif

    glGenTextures(1,&sceneDepthTex);
    glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,dim.width,dim.height,0,GL_DEPTH_COMPONENT,GL_UNSIGNED_SHORT,0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (settings->getIsFOVOn())
    {
        glGenFramebuffers(1,&sceneFOVFramebuffer);
        glGenTextures(1,&sceneFOVTex);
        
        framebuffers.push_back(sceneFOVFramebuffer);

        glBindTexture(GL_TEXTURE_2D, sceneFOVTex);
        glTexImage2D(GL_TEXTURE_2D,0,GL_R8,dim.width,dim.height,0,GL_RED, GL_UNSIGNED_BYTE,0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    #if GRAPHICS_USE_LIGHTING

    glBindFramebuffer(GL_FRAMEBUFFER,sceneFramebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneDiffuseTex, 0);

    if (settings->getIsNormalOn())
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, sceneNormalTex, 0);

    if (settings->getIsSpecularOn())
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, sceneSpecularTex, 0);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sceneDepthTex, 0);

    #endif

    glBindFramebuffer(GL_FRAMEBUFFER,scenePPFramebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, scenePPTex, 0);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sceneDepthTex, 0);

    if (settings->getIsFOVOn())
    {
        glBindFramebuffer(GL_FRAMEBUFFER,sceneFOVFramebuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneFOVTex, 0);

        //White in the FOV buffer means hidden
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glClearColor(0.0f,0.0f,0.0f,1.0f);
    }


    #if GRAPHICS_USE_LIGHTING==0

    sceneFramebuffer = scenePPFramebuffer;
    sceneDiffuseTex = scenePPTex;

    #endif // GRAPHICS_USE_LIGHTING

    glBindFramebuffer(GL_FRAMEBUFFER,0);

    #endif /* COPPERY_HEADLESS */


    sceneFramebufferTextureInterface = std::make_unique<Texture>();
    sceneFramebufferTextureInterface->overrideSource(sceneDiffuseTex, sceneNormalTex, sceneSpecularTex);

    sceneFramebufferDepthTextureInterface = std::make_unique<Texture>();
    sceneFramebufferDepthTextureInterface->overrideSource(sceneDepthTex);
    
    scenePPFramebufferTextureInterface = std::make_unique<Texture>();
    scenePPFramebufferTextureInterface->overrideSource(scenePPTex);
}

void GLState::bindDefaultAttribs()
{

    #ifndef COPPERY_HEADLESS

    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        sizeof(float) * 4, (void*)(sizeof(float) * 0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(float) * 4, (void*)(sizeof(float) * 2));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    #endif /* COPPERY_HEADLESS */

}

void GLState::disableDefaultAttribs()
{
    #ifndef COPPERY_HEADLESS
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    #endif /* COPPERY_HEADLESS */
}

void GLState::updateGlobalUniforms()
{
    assert (currentShader != nullptr);

    if (currentShader == nullShader.get())
        return;

    #ifndef COPPERY_HEADLESS

    if (!currentShader->initializedBasicUniforms)
    {
        glUniformMatrix4fv(currentShader->uniforms.ulocMatProjection,1,false,projection.getGLArray());
        glUniform1i(currentShader->uniforms.ulocTexture,0);
        glUniform1i(currentShader->uniforms.ulocTextureNormal,1);
        glUniform1i(currentShader->uniforms.ulocTextureSpecular,2);
        glUniform1i(currentShader->uniforms.ulocTextureDepth,3);
        glUniform1i(currentShader->uniforms.ulocTextureFOV,4);
        glUniform2f(currentShader->uniforms.ulocWindowSize,scaledDimensions.x,scaledDimensions.y);

        currentShader->initializedBasicUniforms = true;
    }
    
    glUniform2f(currentShader->uniforms.ulocCamera,camera.x,camera.y);

    glUniform1f(currentShader->getUniform(CHash("DeltaTime")), deltaTime);


    #endif /* COPPERY_HEADLESS */
}
