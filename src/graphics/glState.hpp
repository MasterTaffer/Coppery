#pragma once

#include <vector>

#include "matrix.hpp"
#include "vector2.hpp"
#include "uniforms.hpp"
#include <memory>

class Texture;
class Shader;
class GFXFlags;

/*! \brief The manager of OpenGL state.
 * 
 * Stores many snippets of information required by the Graphics and other
 * subsystems. Also initializes the OpenGL framebuffers.
 * 
 * This class is also responsible for the Shader uniforms and their status.
 * The helper class ShaderBinder calls \ref updateGlobalUniforms to update
 * shader uniforms.
 */

class GLState
{
    std::unique_ptr<Shader> nullShader;
    std::vector<unsigned int> framebuffers;
public:
    
    //! Pointer to the scene framebuffer texture 
    std::unique_ptr<Texture> sceneFramebufferTextureInterface;
    //! Pointer to the scene framebuffer depth texture 
    std::unique_ptr<Texture> sceneFramebufferDepthTextureInterface;
    //! Pointer to the scene postprocess framebuffer texture 
    std::unique_ptr<Texture> scenePPFramebufferTextureInterface;

    //! Delta time, used for stepping graphical effects
    float deltaTime = 1.0f;

    ///@{
    //! \brief OpenGL names for framebuffers and their textures
    unsigned int sceneFramebuffer; 
    unsigned int sceneFOVFramebuffer;
    unsigned int sceneDiffuseTex; 
    unsigned int sceneNormalTex; 
    unsigned int sceneSpecularTex;
    unsigned int sceneDepthTex; 
    unsigned int sceneFOVTex; 
    unsigned int scenePPFramebuffer;
    unsigned int scenePPTex;
    ///@}
    

    //! OpenGL name for the common quad polygon vertex buffer
    unsigned int quadBuffer;

    //! Pointer to the current shader that is bound
    Shader* currentShader = nullptr;

    Shader* getNullShader() 
    {
        return nullShader.get();
    }
    
    //! Updates all common shader uniforms
    void updateGlobalUniforms();

    //! Initializes the framebuffers
    void initializeFramebuffers(Vector2i dimensions, GFXFlags* settings);

    //! Deinitializes the framebuffers
    void deInitializeFramebuffers();


    //! Current camera position
    Vector2f camera = {0,0};
    
    //! Current position of the observer
    Vector2f observer = {0,0};
    
    //! Current rendering dimensions. May differ from the actual window dimensions
    Vector2f windowDimensions = {0,0};
    
    //! The scaled dimensions of the screen. 
    Vector2f scaledDimensions = {0,0};
    
    //! Current projection matrix
    Matrix4 projection;

    //! Sets the render and scaled dimensions
    void setDimensions(Vector2f dim, Vector2f scaled)
    {
        windowDimensions = dim;
        scaledDimensions = scaled;
    }
    
    
    //! Get framebuffer by index
    int getFramebufferByIndex(size_t index)
    {
        if (index >= framebuffers.size())
            return -1;
        return framebuffers[index];
    }

    //! Bind and enable the default vertex attribs
    void bindDefaultAttribs();
    
    //! Disable the default vertex attribs
    void disableDefaultAttribs();

    //! Constructor
    GLState();
    
    //! Destructor
    ~GLState();
};
