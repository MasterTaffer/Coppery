#pragma once

/*
    Simple struct used for containing shading/blending settings
*/
struct ShadingSettings
{
    int blendFuncSFactor;
    int blendFuncDFactor;
    int blendEquation;
    bool blending;
    bool depthTest;
    ShadingSettings();
    void activate();
    void release();
};