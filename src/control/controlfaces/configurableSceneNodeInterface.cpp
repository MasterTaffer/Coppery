#include "configurableSceneNodeInterface.hpp"
#include "graphics/scene.hpp"
#include "graphics/configurableSceneNode.hpp"
#include "graphics/graphics.hpp"
#include "graphics/graphicsData.hpp"

ConfigurableSceneNode* newNode(Graphics* graphics, SceneNodeKey key)
{
    auto* scene = graphics->getScene();

    ConfigurableSceneNode* csn = new ConfigurableSceneNode(graphics, key);

    scene->insertNode(csn);

    return csn;
}

void traverse(Graphics* graphics, ConfigurableSceneNode* csn)
{
    csn->SceneBranch::traverse();
    
}

void setTraverse(Graphics* graphics, ConfigurableSceneNode* csn, ConfigurableSceneNode::TraverseMethodType tmethod)
{
    if (csn)
    {
        csn->setTraverseMethod(tmethod);
    }
    
}

void setShader(Graphics* graphics, ConfigurableSceneNode* csn, Shader* shader)
{
    if (csn)
    {
        csn->setShader(shader);
    }
}

void setShadingSettings(Graphics* graphics, ConfigurableSceneNode* csn, bool blending, int sfactor, int dfactor, int equation, bool useDepth)
{
    if (csn)
    {
        csn->shadingSettings.blending = blending;
        csn->shadingSettings.blendFuncSFactor = sfactor;
        csn->shadingSettings.blendFuncDFactor = dfactor;
        csn->shadingSettings.blendEquation = equation;
        csn->shadingSettings.depthTest = useDepth;
    }
}


luaL_Reg configurableSceneNode_functions[] =
{
    {"New", LuaClosureWrap(newNode, 1)},
    {"SetTraverseMethod", LuaClosureWrap(setTraverse, 1)},
    {"SetShader", LuaClosureWrap(setShader, 1)},
    {"Traverse", LuaClosureWrap(traverse, 1)},
    {"SetShadingSettings", LuaClosureWrap(setShadingSettings, 1)},
    {0,0}
};
