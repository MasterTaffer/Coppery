#include "configurableSceneNode.hpp"
#include "glUtilities.hpp"

bool ConfigurableSceneNode::traverse()
{
    shadingSettings.activate();
    if (shader)
    {
        ShaderBinder sb(graphics, shader);
        traverseMethod(this);
    }
    else
        traverseMethod(this);	
    shadingSettings.release();
    return false;
}