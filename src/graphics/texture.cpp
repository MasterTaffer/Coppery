#include "texture.hpp"
#include "log.hpp"
#include "graphicsData.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "oGL.hpp"

#include "fileOperations.hpp"

#include <cstdio>

#define TEXTURE_FILENAME_ENDING ".png"

std::string GetTextureNormalFileName(const std::string& fn)
{
    return fn+".n"+TEXTURE_FILENAME_ENDING;
}

std::string GetTextureDiffuseFileName(const std::string& fn)
{
    return fn+TEXTURE_FILENAME_ENDING;
}

std::string GetTextureSpecularFileName(const std::string& fn)
{
    return fn+".s"+TEXTURE_FILENAME_ENDING;

}



void TextureMap::initializeElement(Texture* s)
{
    s->load(assets);
}

void TextureMap::deInitializeElement(Texture* s)
{
    s->unLoad();
}

void TextureMap::deInitializeDefaultTextures()
{

    #ifndef COPPERY_HEADLESS
    glDeleteTextures(1,&nullDiffuse);
    glDeleteTextures(1,&nullNormal);
    #endif
}
void TextureMap::initializeDefaultTextures()
{
    #ifndef COPPERY_HEADLESS
    glGenTextures(1,&nullDiffuse);
    glBindTexture(GL_TEXTURE_2D,nullDiffuse);
    float ddata[4] = {0.3f,0.3f,0.3f,1.0f};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,1, 1, 0, GL_RGBA ,GL_FLOAT, (void*) ddata);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glGenTextures(1,&nullNormal);
    glBindTexture(GL_TEXTURE_2D,nullNormal);
    float ndata[4] = {0.5f,0.5f,1.0f,0.0f};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,1, 1, 0, GL_RGBA ,GL_FLOAT, (void*) ndata);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    #endif

}

TextureData Texture::loadFile(const std::string& file, TextureLoadParameters tld)
{
    unsigned int texid;
    int n;
    Vector2i dim;
    dim.x = 0;
    dim.y = 0;

    size_t len;

    char* fdata = GetFileContentsCopy(file, &len);
    unsigned char *data = nullptr;
    if (fdata)
    {
        data = stbi_load_from_memory((unsigned char*)fdata, len, &dim.width, &dim.height, &n, 4);
        delete[] fdata;
    }

    #ifndef COPPERY_HEADLESS
    glGenTextures(1,&texid);
    glBindTexture(GL_TEXTURE_2D,texid);
    #endif

    if (dim.x == 0 || dim.y == 0)
    {
        Log << "Faled to load texture " << file << Trace(CHash("Warning"));
        float fdata[4] = {1.0f,1.0f,1.0f,1.0f};

        #ifndef COPPERY_HEADLESS
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,1, 1, 0, GL_RGBA ,GL_FLOAT, (void*) fdata);
        #endif
    }
    else
    {
        #ifndef COPPERY_HEADLESS
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim.width, dim.height, 0, GL_RGBA ,GL_UNSIGNED_BYTE, (void*) data);
        #endif
    }
    #ifndef COPPERY_HEADLESS
    
    if (tld.useMipMaps)
        glGenerateMipmap(GL_TEXTURE_2D);
    
    if (tld.useMipMaps)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    if (!tld.clampX)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    if (!tld.clampY)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    #endif

    if (data)
        stbi_image_free(data);

    TextureData td;
    td.dimensions = dim;
    td.id = texid;
    return td;
}

TextureData Texture::loadMemory(Vector2i dim, char* data, TextureLoadParameters tld)
{
    unsigned int texid;

    #ifndef COPPERY_HEADLESS
    glGenTextures(1,&texid);
    glBindTexture(GL_TEXTURE_2D,texid);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim.width, dim.height, 0, GL_RGBA ,GL_UNSIGNED_BYTE, (void*) data);
    
    if (tld.useMipMaps)
        glGenerateMipmap(GL_TEXTURE_2D);
    
    if (tld.useMipMaps)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    if (!tld.clampX)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    if (!tld.clampY)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    #endif

    TextureData td;
    td.dimensions = dim;
    td.id = texid;
    return td;

}


void Texture::load(GraphicsData* assets)
{
    TextureLoadParameters tld;
    tld.clampX = clampX;
    tld.clampY = clampY;
    tld.useMipMaps = useMipMaps;

    bool specular,normal,diffuse;
    specular = normal = diffuse = false;

    std::string nFile = GetTextureNormalFileName(filenamePrefix);
    std::string sFile = GetTextureSpecularFileName(filenamePrefix);
    std::string dFile = GetTextureDiffuseFileName(filenamePrefix);

    specular = FileExists(sFile);
    diffuse = FileExists(dFile);
    normal = FileExists(nFile);


    normalID = assets->textures.getNullNormal();
    diffuseID = assets->textures.getNullDiffuse();
    specularID = assets->textures.getNullSpecular();
    if (!diffuse)
    {
        Log << "Failed to find texture data " << filenamePrefix << Trace(CHash("Warning"));
        return;
    }


    TextureData texDataDif;

    hasDiffuse = true;
    texDataDif = loadFile(dFile,tld);

    diffuseID = texDataDif.id;
    dimensions = texDataDif.dimensions;

    specularID = diffuseID;


    if (normal)
    {
        hasNormal = true;
        TextureData td;
        td = loadFile(nFile,tld);
        normalID = td.id;
    }

    if (specular)
    {
        hasSpecular = true;
        TextureData td;
        td = loadFile(sFile,tld);
        specularID = td.id;
    }
}

void Texture::loadFromMemory(GraphicsData* assets, Vector2i size, char* difp, char* norp, char* spep)
{
    bool specular,normal,diffuse;


    normal = (norp != nullptr);
    diffuse = (difp != nullptr);
    specular = (spep != nullptr);


    dimensions = size;

    normalID = assets->textures.getNullNormal();
    diffuseID = assets->textures.getNullDiffuse();
    specularID = assets->textures.getNullSpecular();

    if (diffuse)
    {
        TextureData texDataDif;

        hasDiffuse = true;
        texDataDif = loadMemory(size,difp);

        diffuseID = texDataDif.id;

        specularID = diffuseID;
    }

    if (normal)
    {
        hasNormal = true;
        TextureData td;
        td = loadMemory(size,norp);
        normalID = td.id;
    }

    if (specular)
    {
        hasSpecular = true;
        TextureData td;
        td = loadMemory(size,spep);
        specularID = td.id;
    }
}



void Texture::unLoad()
{
    #ifndef COPPERY_HEADLESS
    
    if (hasNormal)
        glDeleteTextures(1,&normalID);

    if (hasSpecular)
        glDeleteTextures(1,&specularID);

    if (hasDiffuse)
        glDeleteTextures(1,&diffuseID);

    #endif

    hasDiffuse = false;
    hasSpecular = false;
    hasDiffuse = false;

    normalID = specularID = diffuseID = 0;
}
