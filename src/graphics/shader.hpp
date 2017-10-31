#pragma once
#include "elementMapper.hpp"
#include "uniforms.hpp"
#include <string>
#include <unordered_map>

//! Structure containing information about OpenGL transform feedback
class TransformFeedbackOptions
{
public:
    //! List of transform feedback varyings used
    std::vector<std::string> varyings;
};

class GFXFlags;
class ShaderMap;

/*! \brief Class for single OpenGL GLSL program
 * 
 * Minimum requirements for a complete OpenGL shader is the fragment
 * and vertex shader. Geometry shader is not required.
 */
class Shader
{
    int loadAndCompile(GFXFlags*,std::string,int);


    void load(GFXFlags* settings);
    void unLoad();
    std::string frag,vert,geom;

    bool hasFrag = false;
    bool hasVert = false;
    bool hasGeom = false;
    bool hasProgram = false;

    int pid;

    int fid,vid,gid; //frag id, vert id, geom id

    TransformFeedbackOptions feedbackOptions;
    std::unordered_map<Hash, int> uniformsByHash;
public:

    //! Sets the fragment shader file name
    Shader* addFragShader(const std::string& s)
    {
        frag = s;
        hasFrag = true;
        return this;
    }

    //! Sets the vertex shader file name
    Shader* addVertShader(const std::string& s)
    {
        vert = s;
        hasVert = true;
        return this;
    }

    //! Sets the geometry shader file name
    Shader* addGeomShader(const std::string& s)
    {
        geom = s;
        hasGeom = true;
        return this;
    }

    //! Set program transform feedback varyings
    Shader* setTransformFeedbackVaryings(const std::vector<std::string>& varyings)
    {
        feedbackOptions.varyings = varyings;
        return this;
    }

    //! Gets the OpenGL shader program name associated with this Shader
    unsigned int getProgram()
    {
        return pid;
    }

    //! Gets the location of uniform by Hash
    int getUniform(Hash h) const
    {
        auto it = uniformsByHash.find(h);
        if (it != uniformsByHash.end())
            return it->second;
        return -1;
    }

    //! Boolean value to determine if the basic uniforms were initialized
    bool initializedBasicUniforms = false;
    
    //! The information about hardcoded uniforms
    UniformLocations uniforms;

    friend class ShaderMap;
};

/*! \brief System for containing the Shader objects
 * 
 * Constructs, destructs and builds the shaders.
 */
class ShaderMap : public ElementMapper<Shader>
{
    GFXFlags* settings;
public:
    
    //! Sets the GFXFlags to be used for building the shaders
    void setSettings(GFXFlags* f) {settings = f;}
protected:
    void initializeElement(Shader*);
    void deInitializeElement(Shader*);
};


