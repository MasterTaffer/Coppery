#include "graphicsFunctions.hpp"
#include "control/crossFunctional.hpp"
#include "graphics/graphics.hpp"
#include "graphics/graphicsData.hpp"
#include "graphics/glUtilities.hpp"
#include "graphics/glState.hpp"
#include "graphics/font.hpp"
#include "oGL.hpp"
#include "graphics/scene.hpp"
#include "graphics/vertexBuffer.hpp"

void bindTextureToUnit(Graphics* graphics, Texture* texture, int unit)
{
    if (!texture || unit < 0)
    {
        return;
    }

    #ifndef COPPERY_HEADLESS
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture->getDiffuse());
    #endif
}


void bindTexture(Graphics* graphics, Texture* texture)
{
    TextureBinder::Bind(graphics, texture);
}


Texture* getPPFramebufferTexture(Graphics* graphics)
{
    return graphics->getGLState()->scenePPFramebufferTextureInterface.get();
}

Texture* getFramebufferTexture(Graphics* graphics)
{
    return graphics->getGLState()->sceneFramebufferTextureInterface.get();
}

Texture* getFramebufferDepthTexture(Graphics* graphics)
{
    return graphics->getGLState()->sceneFramebufferDepthTextureInterface.get();
}



Vector2f getFontDimensions(Graphics* graphics, Font* font)
{
    if (font)
        return font->getCharacterDimensions();
    return {0,0};
}


void drawFramebuffer(Graphics* graphics)
{
    Shader* dshader = graphics->getGLState()->currentShader;
    if (!dshader)
        return;

    graphics->drawFramebuffer();
}

void bindFramebuffer(Graphics* graphics, int index)
{
    int fb = 0;
    if (index > 0)
        fb = graphics->getGLState()->getFramebufferByIndex(index);
    if (fb >= 0)
    {
        #ifndef COPPERY_HEADLESS
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        #endif
    }
    else
        Log << "Invalid framebuffer index in BindFramebuffer" << Trace(CHash("Warning"));
}

void drawQuad(Graphics* graphics, Vector2f center, Vector2f size, float depth, float rotation)
{
    Shader* dshader = graphics->getGLState()->currentShader;
    if (!dshader)
        return;

    Drawing::drawQuadRotated(graphics, center, size, rotation, depth);
}

void drawText(Graphics* graphics, Font* font, const char* str, Vector2f pos, float depth, float scale, float width ,bool centering)
{
    Shader* dshader = graphics->getGLState()->currentShader;
    if (!dshader)
        return;

    if (!font)
        return;

    Drawing::printText(graphics, font, str, pos, scale, width, centering, depth);
}


std::vector<float> generateTextBuffer(Graphics* graphics, Font* font, const char* str, float scale)
{
    if (!font)
        return {};

    Font::TextParameters tp;
    tp.scale = scale;
    return font->generateText(str, tp);
}

void setUniform1f(Graphics* graphics, int uniform, float a)
{
    #ifndef COPPERY_HEADLESS
    glUniform1f(uniform, a);
    #endif
}

void setUniform2f(Graphics* graphics, int uniform, float a, float b)
{
    #ifndef COPPERY_HEADLESS
    glUniform2f(uniform, a, b);
    #endif
}

void setUniform3f(Graphics* graphics, int uniform, float a, float b, float c)
{
    #ifndef COPPERY_HEADLESS
    glUniform3f(uniform, a, b, c);
    #endif
}

void setUniform4f(Graphics* graphics, int uniform, float a, float b, float c, float d)
{
    #ifndef COPPERY_HEADLESS
    glUniform4f(uniform, a, b, c, d);
    #endif
}


void setUniform1i(Graphics* graphics, int uniform, int a)
{
    #ifndef COPPERY_HEADLESS
    glUniform1i(uniform, a);
    #endif
}

void setUniform2i(Graphics* graphics, int uniform, int a, int b)
{
    #ifndef COPPERY_HEADLESS
    glUniform2i(uniform, a, b);
    #endif
}

void setUniform3i(Graphics* graphics, int uniform, int a, int b, int c)
{
    #ifndef COPPERY_HEADLESS
    glUniform3i(uniform, a, b, c);
    #endif
}

void setUniform4i(Graphics* graphics, int uniform, int a, int b, int c, int d)
{
    #ifndef COPPERY_HEADLESS
    glUniform4i(uniform, a, b, c, d);
    #endif
}

void setUniformMat4(Graphics* graphics, int uniform, const Matrix4& mat4, bool transpose)
{
    #ifndef COPPERY_HEADLESS
    glUniformMatrix4fv(uniform, 1, transpose, mat4.data);
    #endif
}



static void setViewport(Graphics* graphics, float a, float b, float c, float d)
{
    #ifndef COPPERY_HEADLESS
    glViewport(a, b, c, d);
    #endif
}

static int queryPerformance(lua_State* lua)
{
    Graphics* g = (Graphics*)lua_touserdata(lua, lua_upvalueindex(1));
    if (g == nullptr)
        return 0;
    lua_newtable(lua);
    #ifndef COPPERY_HEADLESS
    
    auto scene = g->getScene();
    size_t i = 0;
    while (true)
    {
        auto p = scene->getPerformanceQuery(i);
        if (p.first == 0)
            break;
        GLint64 obj = 0;
        glGetQueryObjecti64v(p.first, GL_QUERY_RESULT, &obj);
        
        lua_pushinteger(lua,p.second);
        lua_pushinteger(lua,obj);
        lua_settable(lua,-3);
        i+=1;
        
    }
    #endif
        
    return 1;
}


template <typename T, typename meta>
struct LuaGCWrapper
{

    static int gc(lua_State* lua)
    {
        T* fp = (T*)luaL_checkudata(lua, 1, meta::metaTableName);
        if (fp)
            fp->~T();
        return 0;
    }

    static int construct(lua_State* lua)
    {
        T* obj = (T*) lua_newuserdata(lua, sizeof (T));

        obj = new(obj) T();

        luaL_getmetatable(lua, meta::metaTableName);
        lua_setmetatable(lua, -2);
        return 1;
    }
};

struct VertexBufferMeta
{
    constexpr static const char* metaTableName = "Graphics.VertexBuffer";
};


int newBuffer(lua_State* lua)
{
    return LuaGCWrapper<VertexBuffer, VertexBufferMeta>::construct(lua);
}

void vb_load(VertexBuffer* vb, const std::vector<float>& f, size_t index)
{
    vb->load(f.data(), f.size(), index);
}

void drawBuffer(Graphics* g, VertexBuffer* vb, size_t count, size_t index)
{
    if (vb->getName() == 0)
        return;

    #ifndef COPPERY_HEADLESS
    glBindBuffer(GL_ARRAY_BUFFER, vb->getName());

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        sizeof(float) * 4, (void*)(sizeof(float) * 0));

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(float) * 4, (void*)(sizeof(float) * 2));

    glDrawArrays(GL_TRIANGLES, index, count);
    #endif

    g->getGLState()->bindDefaultAttribs();
}

namespace LuaFunctionWrapper
{
    template <>
    struct ArgumentGetter<VertexBuffer*> 
    {
        static VertexBuffer* Get(lua_State* lua, int i)
        {
            VertexBuffer* fp = (VertexBuffer*)luaL_checkudata(lua, i, VertexBufferMeta::metaTableName);
            return fp;
        }
    };
}


void RegisterGraphicsFunctions(Graphics* g, lua_State* lua)
{

    lua_newtable(lua);


    #ifndef COPPERY_HEADLESS
    #define l_def_integer(x, n) lua_pushinteger(lua, x); lua_setfield(lua, -2, n)
    #else
    int base = 1;
    #define l_def_integer(x, n) lua_pushinteger(lua, base); lua_setfield(lua, -2, n); base++
    #endif

    l_def_integer(GL_ZERO, "Zero");
    l_def_integer(GL_ONE, "One");

    l_def_integer(GL_SRC_COLOR, "SrcColor");
    l_def_integer(GL_ONE_MINUS_SRC_COLOR, "OneMinusSrcColor");

    l_def_integer(GL_DST_COLOR, "DstColor");
    l_def_integer(GL_ONE_MINUS_DST_COLOR, "OneMinusDstColor");

    l_def_integer(GL_SRC_ALPHA, "SrcAlpha");
    l_def_integer(GL_ONE_MINUS_SRC_ALPHA, "OneMinusSrcAlpha");

    l_def_integer(GL_DST_ALPHA, "DstAlpha");
    l_def_integer(GL_ONE_MINUS_DST_ALPHA, "OneMinusDstAlpha");

    l_def_integer(GL_FUNC_ADD, "FuncAdd");
    l_def_integer(GL_FUNC_SUBTRACT, "FuncSubtract");
    l_def_integer(GL_FUNC_REVERSE_SUBTRACT, "FuncReverseSubtract");
    l_def_integer(GL_MIN, "Min");
    l_def_integer(GL_MAX, "Max");

    l_def_integer(GL_STATIC_DRAW, "StaticDraw");
    l_def_integer(GL_DYNAMIC_DRAW, "DynamicDraw");

    l_def_integer(GL_ARRAY_BUFFER, "ArrayBuffer");


    lua_setglobal(lua, "GL");

    lua_newtable(lua);

    l_def_integer(SceneGraphOrder::SceneGraphMesh, "Mesh");

    l_def_integer(SceneGraphOrder::SceneGraphFillLight, "FillLight");
    l_def_integer(SceneGraphOrder::SceneGraphLights, "Lights");

    l_def_integer(SceneGraphOrder::SceneGraphLightlessMesh, "LightlessMesh");

    l_def_integer(SceneGraphOrder::SceneGraphAlpha, "Alpha");
    l_def_integer(SceneGraphOrder::SceneGraphAdditive, "Additive");

    l_def_integer(SceneGraphOrder::SceneGraphUI, "UI");
    
    l_def_integer(SceneGraphOrder::SceneGraphPostProcess, "PostProcess");


    lua_setglobal(lua, "NodeOrder");

    #undef l_def_integer


    {
        typedef LuaGCWrapper<VertexBuffer, VertexBufferMeta> vbgc;
    
        luaL_newmetatable(lua, VertexBufferMeta::metaTableName);

        lua_pushliteral(lua, "__gc");
        lua_pushcfunction(lua, vbgc::gc);
        lua_rawset(lua, -3);

        lua_pushliteral(lua, "__index");
        lua_newtable(lua);

        lua_pushliteral(lua, "load");
        lua_pushcfunction(lua, LuaCWrap(vb_load));
        
        lua_settable(lua, -3);

        lua_pushliteral(lua, "reserve");
        lua_pushcfunction(lua, LuaClassMemberWrap(VertexBuffer, reserve));
        lua_settable(lua, -3);

        lua_pushliteral(lua, "free");
        lua_pushcfunction(lua, LuaClassMemberWrap(VertexBuffer, free));
        lua_settable(lua, -3);

        lua_pushliteral(lua, "getName");
        lua_pushcfunction(lua, LuaClassMemberWrap(VertexBuffer, getName));
        lua_settable(lua, -3);

        lua_pushliteral(lua, "setUsage");
        lua_pushcfunction(lua, LuaClassMemberWrap(VertexBuffer, setUsage));
        lua_settable(lua, -3);

        lua_pushliteral(lua, "setTarget");
        lua_pushcfunction(lua, LuaClassMemberWrap(VertexBuffer, setTarget));
        lua_settable(lua, -3);

        lua_pushliteral(lua, "getSize");
        lua_pushcfunction(lua, LuaClassMemberWrap(VertexBuffer, getSize));
        lua_settable(lua, -3);


        lua_rawset(lua, -3);

        lua_pop(lua, 1);
    }
}

void withShader(Graphics* g, Shader* s, CrossFunction<void, Shader*> fun)
{
    if (!s)
        return;
    ShaderBinder bs(g, s);
    fun(s);
}

luaL_Reg graphicsFunctions_functions[] =
{
    {"DrawQuad", LuaClosureWrap(drawQuad, 1)},


    {"DrawText", LuaClosureWrap(drawText, 1)},
    {"GenerateTextBuffer", LuaClosureWrap(generateTextBuffer, 1)},

    {"NewBuffer", newBuffer},
    {"DrawBuffer", LuaClosureWrap(drawBuffer, 1)},

    {"BindFramebuffer", LuaClosureWrap(bindFramebuffer, 1)},
    {"DrawFramebuffer", LuaClosureWrap(drawFramebuffer, 1)},
    {"BindTexture", LuaClosureWrap(bindTexture, 1)},
    {"BindTextureToUnit", LuaClosureWrap(bindTextureToUnit, 1)},
    {"WithShader", LuaClosureWrap(withShader, 1)},
    {"GetFontDimensions", LuaClosureWrap(getFontDimensions, 1)},
    {"QueryPerformance", queryPerformance},

    {"SetUniform1f", LuaClosureWrap(setUniform1f, 1)},
    {"SetUniform2f", LuaClosureWrap(setUniform2f, 1)},
    {"SetUniform3f", LuaClosureWrap(setUniform3f, 1)},
    {"SetUniform4f", LuaClosureWrap(setUniform4f, 1)},

    {"SetUniform1i", LuaClosureWrap(setUniform1i, 1)},
    {"SetUniform2i", LuaClosureWrap(setUniform2i, 1)},
    {"SetUniform3i", LuaClosureWrap(setUniform3i, 1)},
    {"SetUniform4i", LuaClosureWrap(setUniform4i, 1)},

    {"SetViewport", LuaClosureWrap(setViewport, 1)},
    {"GetFramebufferTexture", LuaClosureWrap(getFramebufferTexture, 1)},
    {"GetFramebufferDepthTexture", LuaClosureWrap(getFramebufferDepthTexture, 1)},
    {"GetPPFramebufferTexture", LuaClosureWrap(getPPFramebufferTexture, 1)},
    
    {0,0}
};
