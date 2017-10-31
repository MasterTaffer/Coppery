#pragma once
#include "elementMapper.hpp"
#include <string>
#include "vector2.hpp"

std::string GetTextureNormalFileName(const std::string&);
std::string GetTextureDiffuseFileName(const std::string&);
std::string GetTextureSpecularFileName(const std::string&);

class TextureLoadParameters
{
public:
    bool clampX = false;
    bool clampY = false;
    bool useMipMaps = true;
};


class ShaderMap;
class GraphicsData;

//! Structure for single OpenGL texture
class TextureData
{
public:
    //! Texture dimensions
    Vector2i dimensions;
    //! Texture OpenGL name
    unsigned int id;
};

/*! \brief Material texture

    Can include multiple OpenGL textures, for diffuse, normal and specular
    channels
*/
class Texture
{
    static TextureData loadFile(const std::string&, TextureLoadParameters tld = TextureLoadParameters());
    static TextureData loadMemory(Vector2i, char*, TextureLoadParameters tld = TextureLoadParameters());

    bool hasNormal = false, hasDiffuse = false, hasSpecular = false;

    bool useMipMaps = true;
    bool clampX = false;
    bool clampY = false;

    void load(GraphicsData*);

    void unLoad();
    Vector2i dimensions = Vector2i(0, 0);
    unsigned int diffuseID, specularID, normalID;
    std::string filenamePrefix;

    void loadFromMemory(GraphicsData*, Vector2i size, char* diffuse, char* normal, char* specular);

public:

    //! Override the texture sources
    void overrideSource(unsigned int diffuse, unsigned int normal = 0, unsigned int specular = 0)
    {
        unLoad();

        diffuseID = diffuse;
        specularID = specular;
        normalID = normal;

        //The texture doesn't own the overriden sources
        hasDiffuse = false;
        hasSpecular = false;
        hasNormal = false;
    }

    //! Return the "official" dimension of the texture
    Vector2i getDimensions()
    {
        return dimensions;
    }

    //! Set the filename of the texture (without extension)
    Texture* setFilename(const std::string& s)
    {
        filenamePrefix = s;
        return this;
    }

    Texture* setClampX(bool cx)
    {
        clampX = cx;
        return this;
    }

    Texture* setClampY(bool cy)
    {
        clampY = cy;
        return this;
    }
    
    Texture* setUseMipMaps(bool use)
    {
        useMipMaps = use;
        return this;
    }

    unsigned int getSpecular()
    {
        return specularID;
    }


    unsigned int getNormal()
    {
        return normalID;
    }


    unsigned int getDiffuse()
    {
        return diffuseID;
    }


    friend class TextureMap;
    friend class TextureSheet;
};

//! Collection of Texture instances
class TextureMap : public ElementMapper<Texture>
{
    GraphicsData* assets = nullptr;
    unsigned int nullDiffuse, nullNormal;
public:
    
    unsigned int getNullDiffuse() {return nullDiffuse;};

    
    unsigned int getNullSpecular() {return nullDiffuse;};

    
    unsigned int getNullNormal() {return nullNormal;};

    Texture* getNull() {return ElementList[0];}

    
    void setAssets(GraphicsData* g) {assets = g;};


    void deInitializeDefaultTextures();


    void initializeDefaultTextures();
protected:
    void initializeElement(Texture*);
    void deInitializeElement(Texture*);
};
