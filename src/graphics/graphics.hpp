
#pragma once
#include "game/engineDefs.hpp"

#include "hash.hpp"
#include "log.hpp"
#include "color.hpp"
#include "config.hpp"
#include "shades.hpp"

#include <map>
#include <set>
#include <list>
#include <random>
#include <vector>
#include <unordered_map>

class Window;
class Drawable;
class SpriteParticleSystem;
class Particle;
class GFXFlags;
class SceneMasterBranch;
class SceneFillLight;
class ParticleSystem;
class GLState;
class GraphicsData;
class Shader;
class GameVariableManager;

struct RegParticleSystemParameters;
struct ParticleSystemParameters
{
    std::string name;
};

/*! \brief System for the graphical output aspects of the application
 * 
 * On the lower level, handles the required OpenGL operations for displaying
 * polygons. On the higher level, handles graphical assets.
 */
class Graphics
{
    std::default_random_engine grandom;
    
    GLState* gl= nullptr;
    GraphicsData* data= nullptr;
    Window* window= nullptr;
    GFXFlags* flags= nullptr;
    SceneFillLight* playerLight= nullptr;
    SceneMasterBranch* scene = nullptr;
    GameVariableManager* variableManager = nullptr;

    int frameCounter = 0;
    double minFrameTime = 0.0;
    double intervalTimer = 0.0;
    double framesPerSecond = 0.0;
    float graphicsDeltaTime = 1.0f;

    //Screen size in different units:

    //Render size: size of framebuffer in pixels. Might not be equal to window size.
    Vector2i renderDimensions;

    //Scaled size: size of viewport in scaled engine dimensions. 
    //can be used to zoom in graphics without decreasing rendering dimensions
    Vector2f scaledDimensions;

    Vector2f windowToCameraScale;
    float scaleFactor;

    std::unordered_map<Hash, ParticleSystem*> particleSystems;
    void updateDrawDimensions();

public:

    //! Request all particle systems to be cleared 
    void clearParticles();

    //! Get a particle system by name
    ParticleSystem* getParticleSystem(Hash h);

    SceneMasterBranch* getScene() {return scene;}

    //! Blit framebuffer
    void drawFramebuffer();

    //! Get graphics delta time
    float getDeltaTime() {return graphicsDeltaTime;}

    //! Set graphics delta time
    void setDeltaTime(float f);

    //! Get random in range 0.0 - 1.0
    float getGRandom();

    //! Get current frames per second
    double getFPS(){return framesPerSecond;};

    //TODO: remove
    //! Get minimum frame time
    double getMinFrameTime(){return minFrameTime;}

    //! Get reference to GraphicsData
    GraphicsData* getAssets(){return data;}

    //! Get reference to GLState
    GLState* getGLState(){return gl;}

    //! Get reference to Window
    Window* getWindow(){return window;}

    //! Get reference to GFXFlags
    GFXFlags* getFlags(){return flags;}

    //! Get refererence to VariableManager
    GameVariableManager* getVariableManager(){return variableManager;}

    //! Get world coordinates of the top left corner of the window
    Vector2f getTopLeft();

    //! Get world coordinates of the top bottom right of the window
    Vector2f getBottomRight();

    //! Set camera center
    void setCameraCenter(Vector2f);

    //! Get camera center
    Vector2f getCameraCenter();

    //! Get mouse position
    Vector2d getMousePosition();

    //! Get mouse position in world space
    Vector2d getCameraMousePosition();

    //! Get size of the framebuffer
    Vector2i getRenderDimensions() {return renderDimensions;};

    //! Get world size of the framebuffer
    Vector2f getScaledDimensions() {return scaledDimensions;};

    //! Get rendering scale factor 
    float getScaleFactor() {return scaleFactor;};

    //! Add a Drawable to the SceneGraph
    void addDrawable(Drawable* d, int layer = SceneGraphOrder::SceneGraphMesh);

    //! Remove a Drawable from the SceneGraph
    void deleteDrawable(Drawable* d);

    //! Initialize Graphics
    void init(GameVariableManager* var, Window*);

    //! Update and draw graphics
    void update();

    //! Deinitialize Graphics
    void deInit();

    //Particle systems

    //! Register external particle system (deprecated)
    bool registerParticleSystem(RegParticleSystemParameters*);

    //! Register generic particle system
    bool registerParticleSystem(ParticleSystem*, ParticleSystemParameters*);

    //! Spawn new particle
    void spawnParticles(Hash system, Vector2f position, Vector2f velocity, Color data, int count);

};

