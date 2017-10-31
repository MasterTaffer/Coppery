
#include <cassert>
#include "oGL.hpp"

#include "drawableTilemap.hpp"
#include "graphics/gfxFlags.hpp"
#include "game/tilemap.hpp"
#include "graphics/texture.hpp"
#include "graphics/graphics.hpp"
#include "graphics/graphicsData.hpp"
#include "graphics/glUtilities.hpp"
#include "graphics/glState.hpp"
#include "graphics/gfxFlags.hpp"

#include <algorithm>


DrawableTilemap::DrawableTilemap(Graphics* g)
{
    graphics = g;

    //TODO: 
    sheet = g->getAssets()->sheets.getElement(CHash("map1"));
    shader = g->getAssets()->shaders.getElement(CHash("texturedMap"));
    
    constructFromTilemap(nullptr, {0,0}, 0.0);

}

void DrawableTilemap::constructFromTilemap(TilemapLayer* tmap, DefVector2 tsize, float d)
{
    
    freeBuffer();
    
    tilemap = ReferenceHolder<TilemapLayer>::create(tmap);
    
    if (tilemap && tilemap->tileset && tilemap->validate() && sheet)
    {
        tileTransformation = TileTransformation(tilemap->tileset->defs, sheet->nameMap, 0);
        assert(tileTransformation.invalid == false);
     
        #ifndef COPPERY_HEADLESS
            glGenBuffers(1,&vbo);
        #else
            vbo = 1;
        #endif   
        
        mapSize = tmap->getSize();
        tileSize = tsize;
        depth = d;

        updateAllTiles();
    }
    else
    {
        mapSize = {0, 0};
        tileSize = {0.0f, 0.0f};
        depth = 0.0;
    }
}


//vec must point to an array of floats with size of at least 20 * sizeof(float)
void DrawableTilemap::writeTileData(Vector2i til, float* vec)
{
    int t = tilemap->getTile(til);
    
    unsigned int id = tileTransformation.data[t];

    Vector2f base, end;
    Vector2f ubase = sheet->getUVBase(id);
    Vector2f usize = sheet->getUVSize();
    Vector2f uend = ubase + usize;

    if (id == 0)
    {
        base = { 0,0 };
        end = { 0,0 };

    }
    else
    {
        base = til * tileSize;
        end = (til + Vector2i(1,1)) * tileSize;
    }

    *(vec++) = (base.x);
    *(vec++) = (base.y);
    *(vec++) = (depth);

    *(vec++) = (ubase.x);
    *(vec++) = (ubase.y);

    *(vec++) = (base.x);
    *(vec++) = (end.y);
    *(vec++) = (depth);

    *(vec++) = (ubase.x);
    *(vec++) = (uend.y);

    *(vec++) = (end.x);
    *(vec++) = (end.y);
    *(vec++) = (depth);

    *(vec++) = (uend.x);
    *(vec++) = (uend.y);

    *(vec++) = (end.x);
    *(vec++) = (base.y);
    *(vec++) = (depth);

    *(vec++) = (uend.x);
    *(vec++) = (ubase.y);
}


void DrawableTilemap::updateAllTiles()
{
    std::vector<float> verts;
    verts.resize(mapSize.x * mapSize.y * 5 * 4);

    size_t offset = 0;
    size_t tilesize = 5 * 4;
    for (int j = 0; j < mapSize.y; j++)
    for (int i = 0; i < mapSize.x; i++)
    {
        writeTileData(Vector2i(i,j), verts.data() + offset);
        offset += tilesize;
    }
    
    #ifndef COPPERY_HEADLESS
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*verts.size(), &verts[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    #endif
}

void DrawableTilemap::freeBuffer()
{
    if (vbo)
    {
        #ifndef COPPERY_HEADLESS
        glDeleteBuffers(1,&vbo);
        #endif
        vbo = 0;
    }
}

DrawableTilemap::~DrawableTilemap()
{
    freeBuffer();
}


void DrawableTilemap::setUniforms(const UniformMap& new_uniforms)
{
    uniforms = UniformMap(new_uniforms, shader);
}

void DrawableTilemap::draw(Graphics* g)
{
    if (!vbo || !sheet)
        return;

    g->getGLState()->disableDefaultAttribs();

    Vector2f offset = Vector2f(0,0);
    (void)(offset); //TODO

    //Calculate the first and last tile on view
    Vector2i firstOnView = (g->getTopLeft()) / tileSize;
    Vector2i lastOnView = (g->getBottomRight()) / tileSize;

    lastOnView += Vector2i(1, 1);

    firstOnView.x = std::max(firstOnView.x, 0);
    firstOnView.y = std::max(firstOnView.y, 0);

    lastOnView.x = std::min(lastOnView.x, mapSize.x);
    lastOnView.y = std::min(lastOnView.y, mapSize.y);


    


    if (shader)
    {
        ShaderBinder sbinder(g, shader);
        uniforms.apply();

        
        int vertsize = sizeof(float)*(3+2);
        int vertspertile = 4;


        #ifndef COPPERY_HEADLESS
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);



        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,vertsize,0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,vertsize,(void*) (sizeof(float)*3));

        #endif

        TextureBinder texturer(g,sheet->getTexture());
        Vector2i dim = mapSize;


        for (int j = firstOnView.y; j < lastOnView.y; j++)
        {
            int x = firstOnView.x;
            int size = lastOnView.x - firstOnView.x;

            int offset = (vertspertile * dim.x * j);
            offset += vertspertile * x;


            #ifndef COPPERY_HEADLESS
                glDrawArrays(GL_QUADS,offset,size*vertspertile);
            #endif

        }

        #ifndef COPPERY_HEADLESS
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        #endif
    }


    g->getGLState()->bindDefaultAttribs();

}
