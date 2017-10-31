#pragma once
#include "elementMapper.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include "vector2.hpp"
class FontData;
class GraphicsData;
class Texture;



/*! \brief A class for encapsulating the features of a font
 * 
 * At this point, Font supports only 256 character monospace
 * bitmap fonts.
 */
class Font
{
    void load(GraphicsData*);
    void unLoad();
    std::string textureName;
    Texture* texture;
    Vector2f characterDimensions;
    Vector2f fontSize;
    
    //std::vector<unsigned int> generatedBuffers;
    //std::unordered_map<unsigned int, unsigned int> bufferKeys;
public:
    
    
    /*
    struct GlyphVertices
    {
        Vector2f uvStart; 
        Vector2f uvEnd;
        Vector2f start;
        Vector2f end;
    };
    */

    
    struct TextParameters
    {
        float scale = 1.0;
    };

    
    
    
    //long generateBuffer(const std::vector<GlyphVertices>& buf);
    
    std::vector<float> generateText(const char* string, const TextParameters& parameters) const;

    //! Gets the upper-left UV of the character
    Vector2f getUV(char c) const;

    //! Gets the size of a single character
    Vector2f getCharacterDimensions() const
    {
        return characterDimensions;
    }

    //! Gets the size of the whole font texture
    Vector2f getFontSize() const
    {
        return fontSize;
    }

    //! Gets the font texture
    Texture* getTexture() const
    {
        return texture;
    }

    //! Sets the font texture asset key to be used
    Font* setTextureName(const std::string& s)
    {
        textureName = s;
        return this;
    }


    friend class FontMap;
};

/*! \brief Collection of all Font instances.
 * 
 * 
 * Takes care of initializing and deinitializing all Font instances.
 */
class FontMap : public ElementMapper<Font>
{
    GraphicsData* assets;
public:
    //! Sets the GraphicsData used to search for font textures
    void setAssets(GraphicsData* g) {assets = g;};
protected:
    void initializeElement(Font*);
    void deInitializeElement(Font*);
};
