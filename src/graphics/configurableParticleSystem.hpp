#pragma once
#include <array>
#include <vector>
#include <memory>

#include "graphics/particle.hpp"
#include "graphics/drawable.hpp"
class Shader;

struct ConfigurableParticleLayout
{
    enum Binding
    {
        Constant = 0,
        Position,
        Velocity,
        Random
    };
    int size = 1;
    int offset = 0;
    Binding binding = Constant;
    float constantValue = 0.0;
    float randomMin = -1.0;
    float randomMax = 1.0;
};

struct ConfigurableParticleSystemParameters
{
    std::vector<ConfigurableParticleLayout> layout;
    std::vector<int> drawLayout;
};

class ConfigurableParticleSystem;

class ConfigurableParticleSystemDrawer : public Drawable
{
    Shader* shader = nullptr;
    ConfigurableParticleSystem* master;
public:
    void release();
    ~ConfigurableParticleSystemDrawer();
    ConfigurableParticleSystemDrawer(ConfigurableParticleSystem*);
    void draw(Graphics*);
    void setShader(Shader* s)
    {
        shader = s;
    }
};


class ConfigurableParticleSystem : public ParticleSystem
{
    Shader* updateShader = nullptr;
    Shader* drawShader = nullptr;
    size_t particleSize = 0;
    size_t maxParticles = 2048;
    

    bool buffersInitialized = false;

    static const unsigned int bufferCount = 3;
    static const unsigned int tfObjectCount = 2;
    std::array<unsigned int, bufferCount> buffers;
    std::array<unsigned int, tfObjectCount> tfObjects;

    size_t newParticles = 0;

    //Data array for new particles, always fits max particle count of data
    std::vector<char> newParticlesData;

    ConfigurableParticleSystemParameters parameters;

    bool isFirstPass = false;

    void enableAttribs(bool update);
    void disableAttribs(bool update);
    Graphics* graphics;

    std::vector<std::unique_ptr<ConfigurableParticleSystemDrawer>> drawers;

public:

    ConfigurableParticleSystemDrawer* addDrawer(int layer);

    void configure(Graphics* g, Shader* updateShader, Shader* drawShader, const ConfigurableParticleSystemParameters& cpsp);
    void release();

    bool update(Graphics* g) override;
    void draw(Graphics* g) override;
    void clear() override;
    void addParticle(Vector2f pos, Vector2f vel) override;

    virtual ~ConfigurableParticleSystem();

    friend class ConfigurableParticleSystemDrawer;
};
