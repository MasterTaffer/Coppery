#include "shadingSettings.hpp"
#include "oGL.hpp"

ShadingSettings::ShadingSettings()
{
    #ifndef COPPERY_HEADLESS
    blendFuncSFactor = GL_ONE;
    blendFuncDFactor = GL_ZERO;
    blendEquation = GL_FUNC_ADD;
    #endif
    blending = false;
    depthTest = true;
}

void ShadingSettings::activate()
{
    #ifndef COPPERY_HEADLESS
    if (blending)
    {
        glEnable(GL_BLEND);
        glBlendFunc(blendFuncSFactor, blendFuncDFactor);
        glBlendEquation(blendEquation);
    }
    if (!depthTest)
        glDisable(GL_DEPTH_TEST);
    #endif
}

void ShadingSettings::release()
{
    #ifndef COPPERY_HEADLESS
    if (!depthTest)
        glEnable(GL_DEPTH_TEST);
    if (blending)
    {
        glDisable(GL_BLEND);
    }
    #endif
}


