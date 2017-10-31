#include "graphicsData.hpp"
#include "shader.hpp"
#include "separatedFile.hpp"
#include "fileOperations.hpp"

#include <string>
#include <sstream>
#include <regex>


void AddFolderToSpriteSheet(TextureSheet* ts, std::string folder)
{
    std::vector<std::string> ret;
    GetFilesInDirectoryRecursive(folder,"png",ret);

    for (std::string a : ret)
    {
        if (a.substr(a.size()-6) == ".n.png")
            continue;
        if (a.substr(a.size()-6) == ".s.png")
            continue;
        std::string c = GetPathWithoutExtension(a);
        std::string b = GetFileNameWithoutExtension(a);

        ts->addFile(c,b);
    }
}

void LoadFolderWithPrefix(TextureMap& tex, std::string folder, std::string prefix)
{
    std::vector<std::string> ret;
    GetFilesInDirectoryRecursive(folder,"png",ret);
    for (std::string a : ret)
    {
        std::string c = GetPathWithoutExtension(a);
        std::string b = GetFileNameWithoutExtension(a);

        tex.addElement(Hash(prefix+b))->setFilename(c);
    }
}


void GraphicsData::loadCharacterAnimations(std::string name, std::string folder)
{
    std::string datafol = AssetsFolder+"/sets/"+folder;
    std::vector<std::string> images;
    GetFilesInDirectoryRecursive(datafol,".png",images);
    for (auto & i : images)
    {
        std::string sk = GetFileNameWithoutExtension(i);
        textures.addElement(Hash(name+"."+sk))->setFilename(GetPathWithoutExtension(i));
    }

    characterAnimations.addElement(Hash(name))->setAnimationName(name)->setAnimationFolder(datafol+"/");
}

void GraphicsData::define()
{

    auto* p = sheets.addElement(CHash("map1"))->setBaseSize(Vector2i(16,16))->addFile(AssetsFolder+"/visThrough","_clear");

    AddFolderToSpriteSheet(p,AssetsFolder+"/ground");

    

    SeparatedFile textureFile = SeparatedFile::BatchLoad(AssetsFolder+"/def","tex",':');
    for (auto sr : textureFile.rows)
    {
        textures.addElement(Hash(sr.getString(0)))->setFilename(AssetsFolder+"/"+sr.getString(1))
        ->setClampX(sr.getInt(2,0))->setClampY(sr.getInt(3,0));;
    }

    LoadFolderWithPrefix(textures, AssetsFolder+"/props","prop");

    SeparatedFile charFile = SeparatedFile::BatchLoad(AssetsFolder+"/def","anim",':');
    for (auto sr : charFile.rows)
    {
        loadCharacterAnimations(sr.getString(0),sr.getString(1));
    }


    SeparatedFile fontFile = SeparatedFile::BatchLoad(AssetsFolder+"/def","font",':');
    for (auto sr : fontFile.rows)
    {
        fonts.addElement(Hash(sr.getString(0)))->setTextureName(sr.getString(1));
    }

}

void GraphicsData::init(GFXFlags* settings)
{

    shaders.setSettings(settings);
    shaders.init();

    textures.initializeDefaultTextures();
    textures.setAssets(this);
    textures.init();

    sheets.setAssets(this);
    sheets.init();

    fonts.setAssets(this);
    fonts.init();

    characterAnimations.setAssets(this);
    characterAnimations.init();

}
void GraphicsData::deInit()
{

    sheets.deInit();

    shaders.deInit();

    textures.deInitializeDefaultTextures();
    textures.deInit();

    fonts.deInit();

    characterAnimations.deInit();
}
