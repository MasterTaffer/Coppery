#pragma once

/*! \brief The hardcoded uniform map
 * 
 * needs* values indicate the presence of the uniform in the shader
 * uloc* is the GL uniform location
 */
class UniformLocations
{
public:
    bool needsMatProjection;
    bool needsTexture;
    bool needsTextureNormal;
    bool needsTextureSpecular;
    bool needsTextureDepth;
    bool needsTextureFOV;
    bool needsCamera;
    bool needsLightPos;
    bool needsLightColor;
    bool needsLightPower;
    bool needsFOVCaster;
    bool needsFadeColor;
    bool needsDefaultNormal;
    bool needsFillAmount;
    bool needsLightFlags;
    bool needsSpecularData;
    bool needsSize;
    bool needsWindowSize;
    
    int ulocMatProjection;
    int ulocTexture;
    int ulocTextureNormal;
    int ulocTextureSpecular;
    int ulocTextureDepth;
    int ulocTextureFOV;
    int ulocCamera;
    int ulocLightPos;
    int ulocLightColor;
    int ulocLightPower;
    int ulocFOVCaster;
    int ulocFadeColor;
    int ulocDefaultNormal;
    int ulocFillAmount;
    int ulocLightFlags;
    int ulocSpecularData;
    int ulocSize;
    int ulocWindowSize;
};
