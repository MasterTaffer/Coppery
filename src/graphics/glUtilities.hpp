#pragma once
#include "hash.hpp"
#include "color.hpp"
#include "vector2.hpp"
#include "matrix.hpp"

class Graphics;
class Texture;
class Shader;
class Font;
class GLState;

/*! \brief RAII style texture binder.
*/
class TextureBinder
{
public:
    TextureBinder(Graphics*,Hash);

    TextureBinder(Graphics*,Texture*);

    ~TextureBinder();

    //Static function to just bind a texture
    static void Bind(Graphics*, Texture*);

};




/*! \brief RAII style shader binder. Automatically restores previous shader
 *  upon destruction.
 */

class ShaderBinder
{
    bool active = false;
    Shader* oldShader;
    GLState* state;
public:

    
    ShaderBinder(Graphics*,Hash);

    ShaderBinder(Graphics*,Shader*);

    ~ShaderBinder();
};

class ProjectionMatrixBinder
{
    Matrix4 oldMatrix;
    GLState* state;
public:
    ProjectionMatrixBinder(Graphics*, const Matrix4& m4);
    ~ProjectionMatrixBinder();
};

/*! \brief Drawing functions
*/
class Drawing
{
public:

    /*! \brief Print text in the most naive fashion
     * 
     * This method shoudln't probably be used when printing a lot of text:
     * the method calculates proper UV coordinates and character positions
     * every time this function is called
     * 
     * \param g graphics
     * \param f font to use
     * \param s text that is written
     * \param pos the top left position of the first character
     * \param scale text scale, where 1.0 is the normal size
     * \param centered if text should be centered
     * \param depth depth used to draw the text
     */
    static void printText(Graphics* g ,Font* f, const char* s ,Vector2f pos,float scale = 1.0f, float width = -1.0f, bool centered = false, float depth = -100.0f);

    /*! \brief Draw a rectangle 

        \param g graphics
        \param pos center
        \param size size
        \param depth depth
    */
    static void drawQuad(Graphics* g,Vector2f pos,Vector2f size, float depth = 0.0f);

    static void drawQuadColored(Graphics* g,Vector2f pos,Vector2f size, Color color, float depth = 0.0f);

    static void drawQuadRotated(Graphics* g, Vector2f pos, Vector2f size, float rotation, float depth = 0.0f);
  
    static void drawRectStatic(Graphics* g,Texture* t,Vector2f p1,Vector2f p2, float depth = 0.0f);

    static void drawLine(Graphics* g,Vector2f p1, Vector2f p2, Color c, float depth = 0.0f, float width = 1.0f);

    static void drawLineQuad(Graphics* g,Vector2f pos,Vector2f size, float depth = 0.0f);
};
