#include "graphics/configurableParticleSystem.hpp"
#include "graphics/glUtilities.hpp"
#include "graphics/graphics.hpp"
#include "graphics/glState.hpp"
#include "oGL.hpp"
#include "log.hpp"

void ConfigurableParticleSystem::configure(
    Graphics* g,
    Shader* us,
    Shader* ds,
    const ConfigurableParticleSystemParameters& cpsp)
{
    if (!us)
    {
        return;
    }
    graphics = g;
    updateShader = us;
    drawShader = ds;
    parameters = cpsp;
    int maxOff = 0;
    for (const auto& p : parameters.layout)
    {
        int off = (p.size + p.offset) * sizeof(float);
        if (off > maxOff)
            maxOff = off;
    }
    particleSize = maxOff;
    

    size_t maxParticles = 65536;
    size_t pbufSize = maxParticles * particleSize;


    if (!buffersInitialized)
    {

        #ifndef COPPERY_HEADLESS

        glGenTransformFeedbacks(tfObjects.size(), tfObjects.data());
        glGenBuffers(buffers.size(), buffers.data());

        for (size_t i = 0; i < tfObjects.size(); i++)
        {
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfObjects[i]);
            glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
            glBufferData(GL_ARRAY_BUFFER, pbufSize, 0, GL_DYNAMIC_DRAW);

            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers[i]);
        }

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
        glBufferData(GL_ARRAY_BUFFER, pbufSize, 0, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        #endif

        newParticlesData.resize(pbufSize);

        newParticles = 0;
        buffersInitialized = true;
        isFirstPass = true;
    }

}


void ConfigurableParticleSystem::release()
{
    updateShader = nullptr;
    drawShader = nullptr;
    particleSize = 0;

    for (auto& d : drawers)
    {
        graphics->deleteDrawable(d.get());
        d->release();
    }

    if (buffersInitialized)
    {
        #ifndef COPPERY_HEADLESS
        glDeleteBuffers(buffers.size(), buffers.data());
        glDeleteTransformFeedbacks(tfObjects.size(), tfObjects.data());
        #endif
        buffersInitialized = false;
    }
}

bool ConfigurableParticleSystem::update(Graphics* g)
{
    ShaderBinder uShader(g, updateShader);

    #ifndef COPPERY_HEADLESS

    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfObjects[0]);
    glBeginTransformFeedback(GL_POINTS);

    #endif

    if (isFirstPass == false)
    {

        
        #ifndef COPPERY_HEADLESS
        glBindBuffer(GL_ARRAY_BUFFER, buffers[1]); 
        #endif

        enableAttribs(true);

        #ifndef COPPERY_HEADLESS
        glDrawTransformFeedback(GL_POINTS, tfObjects[1]);
        #endif


        disableAttribs(true);

        #ifndef COPPERY_HEADLESS
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        #endif

        if (newParticles > 0)
        {

            #ifndef COPPERY_HEADLESS
            glBindBuffer(GL_ARRAY_BUFFER, buffers[2]); 
            glBufferSubData(GL_ARRAY_BUFFER, 0, newParticles * particleSize, newParticlesData.data());
            #endif

            enableAttribs(true);

            #ifndef COPPERY_HEADLESS
            glDrawArrays(GL_POINTS, 0, newParticles);	
            #endif

            disableAttribs(true);

            #ifndef COPPERY_HEADLESS
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            #endif

            newParticles = 0;
        }
    }
    else
    {
        isFirstPass = false;
    }

    isFirstPass = false;
    
    #ifndef COPPERY_HEADLESS	
    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);
    #endif

    std::swap(buffers[0], buffers[1]);
    std::swap(tfObjects[0], tfObjects[1]);
    return false;
}

void ConfigurableParticleSystem::draw(Graphics* g)
{
    if (isFirstPass)
        return;

    if (!drawShader)
        return;

    g->getGLState()->disableDefaultAttribs();

    ShaderBinder dShader(g, drawShader);

    #ifndef COPPERY_HEADLESS	
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    #endif

    enableAttribs(false);

    #ifndef COPPERY_HEADLESS
    glDrawTransformFeedback(GL_POINTS, tfObjects[1]);
    #endif

    disableAttribs(false);

    #ifndef COPPERY_HEADLESS	
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    #endif

    g->getGLState()->bindDefaultAttribs();

}

void ConfigurableParticleSystem::clear()
{
    isFirstPass = true;
}

void ConfigurableParticleSystem::addParticle(Vector2f pos, Vector2f vel)
{
    if (buffersInitialized)
    {
        if (newParticles >= maxParticles)
            return;

        char* dataOffset = newParticlesData.data() + (newParticles * particleSize);

        float* dataFloat = (float*)(dataOffset);

        for (const auto& p : parameters.layout)
        {
            if (p.size == 0)
                continue;

            size_t index = p.offset;

            if (p.binding == ConfigurableParticleLayout::Constant)
            {
                for (int i2 = 0; i2 < p.size; ++i2, ++index)
                    dataFloat[index] = p.constantValue;
            }
            else if (p.binding == ConfigurableParticleLayout::Position)
            {
                if (p.size == 1)
                {
                    dataFloat[index] = pos.x;
                }
                else
                {
                    dataFloat[index] = pos.x;
                    dataFloat[index + 1] = pos.y;
                }
            }
            else if (p.binding == ConfigurableParticleLayout::Velocity)
            {
                if (p.size == 1)
                {
                    dataFloat[index] = vel.x;
                }
                else
                {
                    dataFloat[index] = vel.x;
                    dataFloat[index + 1] = vel.y;
                }
            }
            else if (p.binding == ConfigurableParticleLayout::Random)
            {
                for (int i2 = 0; i2 < p.size; ++i2, ++index)
                    dataFloat[index] = graphics->getGRandom() * (p.randomMax - p.randomMin) + p.randomMin;
            }
        }

        newParticles += 1;
    }
}

ConfigurableParticleSystem::~ConfigurableParticleSystem()
{
    release();
}


void ConfigurableParticleSystem::enableAttribs(bool update)
{
    if (update)
    {

        size_t index = 0;
        for (const auto& p : parameters.layout)
        {
            #ifndef COPPERY_HEADLESS
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, p.size, GL_FLOAT,
                GL_FALSE, particleSize, (void*)(sizeof(float) * p.offset));
            #endif

            ++index;
        }
    }
    else
    {
        size_t index = 0;
        for (int i : parameters.drawLayout)
        {
            if (i >= 0 && ((size_t)i) < parameters.layout.size())
            {
                #ifndef COPPERY_HEADLESS
                glEnableVertexAttribArray(index);
                const auto& p = parameters.layout[i];
                glVertexAttribPointer(index, p.size, GL_FLOAT,
                    GL_FALSE, particleSize, (void*)(sizeof(float) * p.offset));
                #endif

                ++index;
            }
        }

    }

}

void ConfigurableParticleSystem::disableAttribs(bool update)
{
    if (update)
    {
        size_t index = 0;
        for (auto f : parameters.layout)
        {
            (void)f;
            #ifndef COPPERY_HEADLESS
            glDisableVertexAttribArray(index);
            #endif
            ++index;
        }

    }
    else
    {
        size_t index = 0;
        for (int i : parameters.drawLayout)
        {
            if (i >= 0 && ((size_t)i) < parameters.layout.size())
            {
                #ifndef COPPERY_HEADLESS
                glDisableVertexAttribArray(index);
                #endif
            }
            ++index;
        }
    }
}

ConfigurableParticleSystemDrawer* ConfigurableParticleSystem::addDrawer(int layer)
{
    ConfigurableParticleSystemDrawer* d = new ConfigurableParticleSystemDrawer(this);
    drawers.push_back(std::unique_ptr<ConfigurableParticleSystemDrawer>(d));

    graphics->addDrawable(d, layer);
    return d;
}

void ConfigurableParticleSystemDrawer::release()
{
    shader = nullptr;
    master = nullptr;
}

ConfigurableParticleSystemDrawer::~ConfigurableParticleSystemDrawer()
{
    release();
}

ConfigurableParticleSystemDrawer::ConfigurableParticleSystemDrawer(ConfigurableParticleSystem* system)
{
    master = system;
}

void ConfigurableParticleSystemDrawer::draw(Graphics* g)
{
    if (shader && master)
    {
        if (master->isFirstPass)
            return;

        g->getGLState()->disableDefaultAttribs();

        ShaderBinder dShader(g, shader);

        #ifndef COPPERY_HEADLESS
        glBindBuffer(GL_ARRAY_BUFFER, master->buffers[1]);
        #endif

        master->enableAttribs(false);

        #ifndef COPPERY_HEADLESS
        glDrawTransformFeedback(GL_POINTS, master->tfObjects[1]);
        #endif

        master->disableAttribs(false);


        #ifndef COPPERY_HEADLESS
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        #endif

        g->getGLState()->bindDefaultAttribs();
    }
}