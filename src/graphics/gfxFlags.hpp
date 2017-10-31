#pragma once

#include <vector>
#include <string>

#include <unordered_map>
#include "hash.hpp"

#define GRAPHICS_USE_LIGHTING 1

class Config;
class GameVariableManager; 

/*! \brief Structure representing graphical settings
*/

class GFXFlags
{
    std::vector<std::string> shaderPrefix;
    bool specular = false;
    bool normal = false;
    bool fov = false;
    bool postProcess = true;
    bool requestingReInitialization = false;
    std::vector<std::string> dataFloatNames;
    std::unordered_map<Hash, float> dataFloat;
public:

    float getDataFloat(Hash h)
    {
        return dataFloat[h];
    };

    void setDataFloat(Hash h, float f)
    {
        dataFloat[h] = f;
    };

    //! Palauttaa, 
    bool getIsRequestingReInitialization() {return requestingReInitialization;}

    /*! \brief Asettaa, grafiikka tarvitsee uudelleen initialisointia

        Grafiikka tarvitsee uudelleen initialisointia, kun framebuffer asetuksia
        halutaan muuttaa, tai shaderit  recompiled.
    */
    void setIsRequestingReInitialization(bool b ) {requestingReInitialization = b;}

    //! updates shadereitten asetukset

    void updateShaderPrefix();

    bool getIsFOVOn() {return fov;}

    bool getIsSpecularOn() {return specular;}

    bool getIsNormalOn() {return normal;}

    bool getIsPostProcessOn() {return postProcess;}

    //! Lataa asetukset tiedostosta
    void loadSettings(GameVariableManager* var);

    //! Palauttaa nykyiset Shader asetukset
    std::vector<std::string>& getShaderPrefix() {return shaderPrefix;};

    bool drawFOV = true; 

    int particles = 1;
};
