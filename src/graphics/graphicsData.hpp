#pragma once
#include "shader.hpp"
#include "texture.hpp"
#include "characterAnimator.hpp"
#include "font.hpp"
#include "sheet.hpp"

//! Contains all the graphical assets

class GraphicsData
{
    void loadCharacterAnimations(std::string name, std::string folder);
public:
    TextureSheetMap sheets;

    TextureMap textures;

    ShaderMap shaders;

    FontMap fonts;

    CharacterAnimationMap characterAnimations;

    void define();

    void init(GFXFlags* settings);

    void deInit();
};
