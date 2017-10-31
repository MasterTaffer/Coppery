#include "font.hpp"
#include "texture.hpp"
#include "graphics/graphicsData.hpp"
#include "graphics/texture.hpp"

void Font::load(GraphicsData* assets)
{
    texture = assets->textures.getElement(Hash(textureName));
    if (texture)
    {
        characterDimensions = Vector2f(texture->getDimensions())/16.f;
        fontSize = Vector2f(texture->getDimensions());	
    }
}

void Font::unLoad()
{

}

Vector2f Font::getUV(char c) const
{
    unsigned char cu = c;
    char row = cu/16;
    char col = cu%16;

    return {float(col)/16.f,float(row)/16.f};
}
void FontMap::initializeElement(Font * f)
{
    f->load(assets);
}

void FontMap::deInitializeElement(Font * f)
{
    f->unLoad();
}

std::vector<float> Font::generateText(const char* string, const TextParameters& parameters) const
{
    Vector2f base {0, 0};
    std::vector<float> ret;
    
    Vector2f dim = getCharacterDimensions() * parameters.scale;

    float initX = base.x;

        
    Vector2f pos = base;
    float xoff = 0.0f;
    pos.x += xoff;
  
    int index = -1;  
    for (const char* it = string; *it; ++it)
    {
        char c = *it;
        index++;

        if (c == '\n')
        {
            pos.x = initX + xoff;
            pos.y += dim.y;
            continue;
        }

        Vector2f uv1 = getUV(c);
        Vector2f uv2 = uv1 + Vector2f(1.f/16.f,1.f/16.f);


        /*
        Font::GlyphVertices gv;
        gv.uvStart = uv1;
        gv.uvEnd = uv2;
        gv.start = pos;
        gv.end = pos + dim;
        */

        Vector2f end = pos + dim;

        auto push_position = [&](const std::pair<float, float>& b)
        {
            ret.push_back(std::get<0>(b));
            ret.push_back(std::get<1>(b));
        };

        auto push_uv = [&](const std::pair<float, float>& b)
        {
            ret.push_back(std::get<0>(b));
            ret.push_back(std::get<1>(b));
        };

        push_position({pos.x, pos.y});
        push_uv({uv1.x, uv1.y});

        push_position({pos.x, end.y});
        push_uv({uv1.x, uv2.y});

        push_position({end.x, pos.y});
        push_uv({uv2.x, uv1.y});

        push_position({end.x, pos.y});
        push_uv({uv2.x, uv1.y});

        push_position({pos.x, end.y});
        push_uv({uv1.x, uv2.y});

        push_position({end.x, end.y});
        push_uv({uv2.x, uv2.y});


        pos.x += dim.x;
    }
    
    return ret;
    
}
