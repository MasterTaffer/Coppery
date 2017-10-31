#pragma once

#include <array>
#include <vector>
#include <memory>

#include "control/crossFunctional.hpp"
#include "graphics/scene.hpp"
#include "graphics/shadingSettings.hpp"

class Shader;



class ConfigurableSceneNode : public SceneBranch
{
public:
    typedef CrossFunction<void, ConfigurableSceneNode*> TraverseMethodType;
protected:
    Shader* shader = nullptr;
    const intptr_t value;
    CrossFunction<void, ConfigurableSceneNode*> traverseMethod;

    virtual SceneNodeBase* createNode(int) override
    {
        return nullptr;
    }

public:
    ShadingSettings shadingSettings;
    
    ConfigurableSceneNode(Graphics* g, intptr_t value) : SceneBranch(g), value(value) {}
    virtual intptr_t getValue() const override  {return value;}
    virtual bool traverse() override;

    void setShader(Shader* s)
    {
        shader = s;
    }

    void setTraverseMethod(CrossFunction<void, ConfigurableSceneNode*> tmethod)
    {
        traverseMethod = tmethod;
    }
};