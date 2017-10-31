#include "graphics/drawable.hpp"
#include <vector>
#include "game/tilemap.hpp"
#include "uniformMap.hpp"

#include "mapTransformer.hpp"

class Texture;
class Shader;
class TextureSheet;


/*! \brief Drawable for Tilemap

*/
class DrawableTilemap : public Drawable
{
    using TileTransformation = MapTransformation<std::unordered_map<Hash, int>, std::unordered_map<Hash, int>, int, int>;
    TileTransformation tileTransformation;
    
    Vector2i mapSize;
    Vector2f tileSize;
    float depth = 0.0;

    TextureSheet* sheet;
    Graphics* graphics;
    Shader* shader;

    UniformMap uniforms;

    unsigned int vbo = 0;
    ReferenceHolder<TilemapLayer> tilemap;

    void writeTileData(Vector2i til, float* data);
    void updateAllTiles();
    void freeBuffer();

public:
    void setUniforms(const UniformMap& uniforms);
    
    void draw(Graphics*);

    void constructFromTilemap(TilemapLayer* tmap, DefVector2 tileSize, float depth);

    /*! \brief Constructor
    */
    DrawableTilemap(Graphics* g);

    virtual ~DrawableTilemap();
};

