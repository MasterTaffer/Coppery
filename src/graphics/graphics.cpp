#include "graphics.hpp"
#include "matrix.hpp"

#include "glState.hpp"
#include "graphicsData.hpp"
#include "oGL.hpp"
#include "glUtilities.hpp"
#include "gfxFlags.hpp"
#include "drawable.hpp"
#include "drawableUI.hpp"
#include "particle.hpp"

#include "configurableParticleSystem.hpp"

#include "scene.hpp"
#include "shades.hpp"

#include "window/window.hpp"

#include <limits>
#include <algorithm>
#include <sstream>
#include "variable.hpp"

Vector2d Graphics::getCameraMousePosition()
{
    return window->getMousePosition()*windowToCameraScale + getGLState()->camera;
}

void Graphics::init(GameVariableManager* var, Window * w)
{
    scaleFactor = 1.0/1.0;
    window = w;

    std::vector<GameVariable*> gvars;
    gvars.push_back(var->makeNumberLimits("Graphics.RenderScale", 0, GameVariable::NumberMax, 1.0));
    gvars.push_back(var->makeNumber("Graphics.MinFrameTime", 1.0, std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            this->minFrameTime = gv.getNumber();
            return true;
        })));

    var->loadFromConfig(gvars, "graphics");

    double renderDiv = var->get(CHash("Graphics.RenderScale"))->getNumber();



    if (renderDiv <= 0.0)
       renderDiv = 1.0;


    renderDimensions = w->getWindowDimensions() / renderDiv;
    scaledDimensions = renderDimensions * scaleFactor;

    windowToCameraScale = Vector2f(1,1)/renderDiv;

    flags = new GFXFlags();
    flags->loadSettings(var);
    flags->updateShaderPrefix();

    gl = new GLState();

    gl->setDimensions(renderDimensions, scaledDimensions);
    gl->projection = Matrix4::calculateOrtho(scaledDimensions, 100, -100);

    data = new GraphicsData();

    //Create the SceneGraph
    scene = new SceneMasterBranch(this);

    //Control Lua is responsible for filling the scene graph


    data->define();
    data->init(flags);


    #ifndef COPPERY_HEADLESS

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    
    glClearColor(0.0,0.0,0.0,0.0);

    intervalTimer = glfwGetTime();
    frameCounter = 0;

    #endif

    gl->initializeFramebuffers(getRenderDimensions(), flags);
}

void Graphics::addDrawable(Drawable* d, int layer)
{
    if (d->node != nullptr)
        deleteDrawable(d);

    if (layer <= 0) //Default to mesh layer
        layer = SceneGraphOrder::SceneGraphMesh;


    auto* branch = scene->getBranch(layer);
    if (branch)
    {
        SceneDrawableLeaf* sdl = new SceneDrawableLeaf(this,d);
        if (!branch->insertNode(sdl))
        {
            //Failed to insert the node
            delete sdl;
        }
        else
            d->node = sdl;
    }
    else
    {
        Log << "Illegal drawable node layer "<< layer << Trace(CHash("Warning"));
    }
}

void Graphics::deleteDrawable(Drawable* d)
{
    SceneDrawableLeaf* sdl = d->node;
    if (sdl != nullptr)
    {
        sdl->drawable = nullptr;
        sdl->setRemoval();
    }
    d->node = nullptr;
}


void Graphics::setDeltaTime(float f)
{
    gl->deltaTime = f;
    graphicsDeltaTime = f;
}

Vector2d Graphics::getMousePosition()
{
    Vector2d msp = window->getMousePosition();
    msp =  msp / window->getWindowDimensions();
    msp = msp * getScaledDimensions();
    return msp;
}

float Graphics::getGRandom()
{
    //Commented code is about 70% slower

    //std::uniform_real_distribution<> dist(0, 1);
    //return dist(grandom);

    //This is good enuff
    return float((grandom() & 0xFFFF00) >> 8 ) / 65536.0;
}

void Graphics::drawFramebuffer()
{
    #ifndef COPPERY_HEADLESS
    //ruudun koko
    Vector2f worldDim = getScaledDimensions();

    glBegin(GL_QUADS);

    glVertexAttrib2f(1,0,1);
    glVertexAttrib2f(2,gl->camera.x,gl->camera.y);
    glVertex3f(-1,1,0);

    glVertexAttrib2f(1,0,0);
    glVertexAttrib2f(2,gl->camera.x,gl->camera.y+worldDim.y);
    glVertex3f(-1,-1,0);

    glVertexAttrib2f(1,1,0);
    glVertexAttrib2f(2,gl->camera.x+worldDim.x,gl->camera.y+worldDim.y);
    glVertex3f(1,-1,0);

    glVertexAttrib2f(1,1,1);
    glVertexAttrib2f(2,gl->camera.x+worldDim.x,gl->camera.y);
    glVertex3f(1,1,0);

    glEnd();

    #endif

}

void Graphics::clearParticles()
{
    for (auto& p : particleSystems)
        p.second->clear();
}

void Graphics::update()
{

    if (flags->getIsRequestingReInitialization())
    {
        flags->updateShaderPrefix();
        gl->deInitializeFramebuffers();
        data->deInit();
        data->init(flags);
        gl->initializeFramebuffers(getRenderDimensions(),flags);

        flags->setIsRequestingReInitialization(false);
    }

    //Update particle systems 
    for (auto& p : particleSystems)
        p.second->update(this);

    if (getFlags()->getIsFOVOn())
    {

        #ifndef COPPERY_HEADLESS
        glBindFramebuffer(GL_FRAMEBUFFER,gl->sceneFOVFramebuffer);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        #endif /* COPPERY_HEADLESS*/
    }


    #ifndef COPPERY_HEADLESS
    glEnable(GL_DEPTH_TEST);

    glViewport(0,0,getRenderDimensions().x,getRenderDimensions().y);

    glBindFramebuffer(GL_FRAMEBUFFER,gl->sceneFramebuffer); 

    //Clear the framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #endif /* COPPERY_HEADLESS*/
    {



        //sceneFrameBuffer has at most 3 surfaces
        //diffuse, normal and specular
        //depending on the settings


        #ifndef COPPERY_HEADLESS

        #if GRAPHICS_USE_LIGHTING

        if (flags->getIsNormalOn())
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            glClearColor(0.5,0.5,1.0,1.0);
            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(0.0,0.0,0.0,0.0);
        }

        unsigned int buf1 = GL_NONE;
        unsigned int buf2 = GL_NONE;
        if (flags->getIsNormalOn())
            buf1 = GL_COLOR_ATTACHMENT1;
        if (flags->getIsSpecularOn())
            buf2 = GL_COLOR_ATTACHMENT2;


        if (!flags->getIsSpecularOn() && !flags->getIsNormalOn())
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }
        else
        {
            unsigned int buffers[] = {GL_COLOR_ATTACHMENT0, buf1, buf2};
            glDrawBuffers(3,buffers);
        }
        #else
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        #endif
        #endif /* COPPERY_HEADLESS*/

        // Default shader
        Shader * s = data->shaders.getElement(CHash("textured"));

        //Bind RAII style
        ShaderBinder shader(this,s);

        getGLState()->bindDefaultAttribs();
        scene->traverse();
        getGLState()->disableDefaultAttribs();

    }

    //FPS Counting

    frameCounter++;

    if (frameCounter == 60)
    {

        #ifndef COPPERY_HEADLESS
        double ntime = glfwGetTime();
        double dif = ntime-intervalTimer;
        framesPerSecond = frameCounter/dif;
        intervalTimer = ntime;
        frameCounter = 0;
        std::stringstream ss;
        ss << "FPS: " << framesPerSecond;
        window->setTitle(ss.str());

        #endif /* COPPERY_HEADLESS*/
    }


    //Check GL Errors

    #ifndef COPPERY_HEADLESS
    bool hadError = false;
    
    while(true)
    {

        auto error = glGetError();
        if (error == GL_NO_ERROR)
            break;
        if (!hadError)
            Log << "Last rendered frame contained errors" << Trace(CHash("GraphicsWarning"));
        hadError = true;
        Log << "Unhandled OpenGL error "<< error << Trace(CHash("GraphicsWarning"));
    }

    #endif /* COPPERY_HEADLESS*/
}

void Graphics::deInit()
{
    
    for (auto p : particleSystems)
    {
        delete p.second;
    }
    particleSystems.clear();


    data->deInit();
    gl->deInitializeFramebuffers();


    scene->clear();

    delete scene;
    delete data;
    delete gl;
    delete flags;

    gl= nullptr;
    data= nullptr;
    window= nullptr;
    flags= nullptr;
    scene = nullptr;
    variableManager = nullptr;
}

Vector2f Graphics::getTopLeft()
{
    return gl->camera;
}
Vector2f Graphics::getBottomRight()
{
    return gl->camera + (getScaledDimensions());
}

void Graphics::setCameraCenter(Vector2f cam)
{
    gl->camera = cam - (getScaledDimensions() / 2);
}

Vector2f Graphics::getCameraCenter()
{
    return gl->camera + (getScaledDimensions() / 2);
}


ParticleSystem* Graphics::getParticleSystem(Hash h)
{
    auto it = particleSystems.find(h);
    if (it != particleSystems.end())
        return it->second;
    return nullptr;
}

bool Graphics::registerParticleSystem(ParticleSystem* p, ParticleSystemParameters* o)
{
    if (particleSystems.find(Hash(o->name)) != particleSystems.end())
    {
        Log << "Failed to add particle system '" << o->name << "' - duplicate name" << Trace(CHash("Warning"));
        return false;
    }
   
    particleSystems[Hash(o->name)] = p;
    return true;
}


void Graphics::spawnParticles(Hash system, Vector2f position, Vector2f velocity, Color data, int count)
{
    auto p = particleSystems.find(system);
    if (p != particleSystems.end())
    {
        if (count == 1)
        {
            p->second->addParticle(position, velocity);
        }
        else
        for (int i = 0; i < count; i++)
        {
            p->second->addParticle(position + Vector2f(getGRandom() * 4.0 - 2.0, getGRandom() * 4.0 - 2.0), velocity * (getGRandom() / 2.0 + 0.5) + Vector2f(getGRandom() * 2.0 - 1.0, getGRandom() * 2.0 - 1.0));
        }
    }
}
