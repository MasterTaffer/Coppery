#include <exception>
#include "window.hpp"

#include "inputCascade.hpp"
#include "keyConfig.hpp"
#include "oGL.hpp"

#include "variable.hpp"

#include <mutex>

//for icon loading
#include <stb_image.h>
#include "fileOperations.hpp"

std::mutex frameCodePointGuard;
std::vector<unsigned int> currentFrameCodePoints;

#ifndef COPPERY_HEADLESS
void charCallback(GLFWwindow* win, unsigned int cp)
{
    std::lock_guard<std::mutex> guard(frameCodePointGuard);
    currentFrameCodePoints.push_back(cp);
}
#endif


double Window::getTime()
{
    #ifndef COPPERY_HEADLESS
    return glfwGetTime();
    #else
    return 0.0;
    #endif
}

void Window::updateInputs()
{
    inputBuffer.clear();

    #ifndef COPPERY_HEADLESS

    anyKeyPressed = false;

    for (KeyInputStyle& i : keyInputs)
    {
        if (i.key == 0)
            continue;

        char val = glfwGetKey(glWindow,i.key);

        if (val == GLFW_PRESS)
            val = 1;
        else
            val = 0;

        if (i.reversed)
            val *= -1;

        inputBuffer.push_back({i.inputIndex, val});
    }

    for (MouseButtonInputStyle& i : mouseButtonInputs)
    {
        char val = glfwGetMouseButton(glWindow, i.key);
        if (val == GLFW_PRESS)
            val = 1;
        else
            val = 0;
        if (i.reversed)
            val *= -1;

        inputBuffer.push_back({i.inputIndex, val});
    }

    for (JoyButtonInputStyle& i: joyButtonInputs)
    {

        int c = 0;
        const unsigned char* d = glfwGetJoystickButtons(i.joy , &c);
        if (d==nullptr)
            continue;
        if (c <= i.button)
            continue;
        char val = d[i.button];
        if (val == GLFW_PRESS)
            val = 1;
        else
            val = 0;
        if (i.reversed)
            val *= -1;

        inputBuffer.push_back({i.inputIndex, val});
    }

    for (JoyAxisInputStyle& i: joyAxisInputs)
    {
        int c = 0;
        const float* d = glfwGetJoystickAxes(i.joy , &c);
        if (d==nullptr)
            continue;
        if (c <= i.axis)
            continue;
        float val =d[i.axis];
        if (i.reversed)
            val *= -1;
        if (fabsf(val) <= joystickDeadZone)
            val = 0.0;

        inputBuffer.push_back({i.inputIndex, val});
    }

    inputCascade->flowInput(inputBuffer);

    #endif /* COPPERY_HEADLESS */
    inputCascade->update();
}


void Window::init(GameVariableManager* var)
{

    #ifndef COPPERY_HEADLESS

    glfwSetErrorCallback([](int error, const char* message)
        {
            Log << "Code " << error << ", " << message << Trace(CHash("Warning"));
        });

    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    #endif 

    GameVariable* gv_width;
    GameVariable* gv_height;
    GameVariable* gv_fullscreen;
    GameVariable* gv_swapinterval;
    std::vector<GameVariable*> gvars;
    gvars.push_back(gv_width = var->makeInteger("Graphics.RenderWidth", 1.0));
    gvars.push_back(gv_height = var->makeInteger("Graphics.RenderHeight", 1.0));
    gvars.push_back(gv_fullscreen = var->makeIntegerLimits("Graphics.FullScreen", 0, 1, 0));
    gvars.push_back(gv_swapinterval = var->makeIntegerLimits("Graphics.SwapInterval", 0, 2, 0));
    gvars.push_back(var->makeNumberLimits("Graphics.JoystickDeadZone", 0.0, 1.0, 0.02, std::function<bool(const GameVariable&)>([this](const GameVariable& gv)
        {
            joystickDeadZone = gv.getNumber();
            return true;
        })));

    var->loadFromConfig(gvars, "graphics");

    

    dimensions.width = gv_width->getInteger();
    dimensions.height = gv_height->getInteger();
    fullscreenAfterRestart = fullscreen = gv_fullscreen->getInteger();
    swapInterval = gv_swapinterval->getInteger();
    const char* windowTitle = "-";

    #ifndef COPPERY_HEADLESS

    glfwWindowHint(GLFW_RESIZABLE,false);
    glfwWindowHint(GLFW_ALPHA_BITS,0);
    glfwWindowHint(GLFW_DEPTH_BITS,0);

    Log << "Starting GLFW with dimensions " << dimensions.x << "x"  << dimensions.y << " in ";
    Log << (fullscreen ? "fullscreen" : "windowed") << " mode" << Trace(CHash("General"));
    if (fullscreen)
        glWindow = glfwCreateWindow(dimensions.width, dimensions.height, windowTitle, glfwGetPrimaryMonitor(), NULL);
    else
        glWindow = glfwCreateWindow(dimensions.width, dimensions.height, windowTitle, NULL, NULL);
 

//    dimensions = fbs;

    if (!glWindow)
    {        
        throw std::runtime_error("Failed to initialize GLFW window");
    }

    glfwSetWindowSize(glWindow,dimensions.x,dimensions.y);

    glfwGetFramebufferSize(glWindow,&outputDimensions.x,&outputDimensions.y);
    
    glfwMakeContextCurrent(glWindow);

    if (ogl_LoadFunctions() == 0)
    {
        throw std::runtime_error("Failed to initialize OpenGL functions (OpenGL LoadGen)");
    }

    glfwSetInputMode(glWindow, GLFW_STICKY_KEYS, 1);


    for (int i = 0; i < GLFW_JOYSTICK_LAST; i++)
    {
        bool present = glfwJoystickPresent(i);
        if (present)
        {
            Log << "Controller "<< i << ": " << glfwGetJoystickName(i) << Trace(CHash("ControllerInfo"));
            int buttons, axes;
            glfwGetJoystickButtons(i,&buttons);
            glfwGetJoystickAxes(i,&axes);
            Log << "Buttons: " << buttons << " Axes: " << axes << Trace(CHash("ControllerInfo"));
        }
    }

    glfwSwapInterval(swapInterval);
    glfwSetCharCallback(glWindow, charCallback);

    #endif /* COPPERY_HEADLESS */


    inputCascade = std::make_unique<InputCascade>();

    keyConfig = new KeyConfig();
    keyConfig->load("user/config/input.cfg");




    mouseButtonInputs = keyConfig->getMouseButtons();
    keyInputs = keyConfig->getKeys();
    joyAxisInputs = keyConfig->getAxes();
    joyButtonInputs = keyConfig->getJoyButtons();

    std::vector<InputStyle*> istyles;
    for (auto& t : keyInputs)
    {
        istyles.push_back(&t);
    }

    for (auto& t : mouseButtonInputs)
    {
        istyles.push_back(&t);
    }

    for (auto& t : joyAxisInputs)
    {
        istyles.push_back(&t);
    }
    for (auto& t : joyButtonInputs)
    {
        istyles.push_back(&t);
    }

    inputCascade->addLayer();
    inputCascade->createInputs(istyles);

/*
    uiInputs[GLFW_KEY_ENTER] = Input();
    uiInputs[GLFW_KEY_BACKSPACE] = Input();
    uiInputs[GLFW_KEY_DELETE] = Input();


    uiInputs[GLFW_KEY_LEFT] = Input();
    uiInputs[GLFW_KEY_RIGHT] = Input();
    uiInputs[GLFW_KEY_UP] = Input();
    uiInputs[GLFW_KEY_DOWN] = Input();

    uiInputs[GLFW_KEY_C] = Input();
    uiInputs[GLFW_KEY_X] = Input();
    uiInputs[GLFW_KEY_V] = Input();

    uiInputs[GLFW_KEY_LEFT_CONTROL] = Input();
    uiInputs[GLFW_KEY_RIGHT_CONTROL] = Input();
    uiInputs[GLFW_KEY_LEFT_SHIFT] = Input();
    uiInputs[GLFW_KEY_RIGHT_SHIFT] = Input();
    uiInputs[GLFW_KEY_LEFT_ALT] = Input();
    uiInputs[GLFW_KEY_RIGHT_ALT] = Input();
*/


    updateVideoModes();


}


void Window::deInit()
{

    //keyConfig->setValues(joyAxisInputs,joyButtonInputs,keyInputs,mouseButtonInputs);
    //keyConfig->write("user/config/input2.cfg");
    delete keyConfig;
    inputCascade.reset();


    #ifndef COPPERY_HEADLESS

    glfwDestroyWindow(glWindow);
    glfwTerminate();

    #endif /* COPPERY_HEADLESS */

    glWindow = nullptr;

}

void Window::update()
{

    #ifndef COPPERY_HEADLESS

    glfwSwapBuffers(glWindow);

    glfwGetFramebufferSize(glWindow, &outputDimensions.x, &outputDimensions.y);

    #endif /* COPPERY_HEADLESS */

    #ifndef COPPERY_HEADLESS
    glfwPollEvents();
    glfwGetCursorPos(glWindow, &mousePos.x, &mousePos.y);
    #endif /* COPPERY_HEADLESS */


    updateInputs();


    {
        std::lock_guard<std::mutex> guard(frameCodePointGuard);
        frameCodePoints = currentFrameCodePoints;
        currentFrameCodePoints.clear();
    }
}

std::vector<Vector2i>& Window::getVideoModes()
{
    return videoModes;
}

void Window::setNewVideoMode(Vector2i d)
{
    videoModeAfterRestart = d;
}

void Window::updateVideoModes()
{
    int count = 0;

    #ifndef COPPERY_HEADLESS

    const GLFWvidmode* vidModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
    videoModes.clear();
    for (int i = 0; i < count; i++)
    {
        Vector2i comp = {vidModes[i].width,vidModes[i].height};

        bool found = false;
        for (int j = 0; j < int(videoModes.size()); j++)
        {
            if (videoModes[j] == comp)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;
        videoModes.push_back(comp);
    }

    #endif /* COPPERY_HEADLESS */

    std::vector<Vector2i> commonVideoModes;
    commonVideoModes.push_back({800,600});
    commonVideoModes.push_back({1024,768});
    commonVideoModes.push_back({1280,720});

    for (auto& vmd : commonVideoModes)
    {
        bool found = false;
        for (auto& cmp : videoModes)
        {
            if (cmp == dimensions)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;
        videoModes.push_back(vmd);
    }	

    currentVideoModeIndex = -1;
    for (int j = 0; j < int(videoModes.size()); j++)
    {
        if (videoModes[j] == dimensions)
        {
            currentVideoModeIndex = j;
            break;
        }
    }

    if (currentVideoModeIndex == -1)
    {
        videoModes.push_back(dimensions);
        currentVideoModeIndex = videoModes.size()-1;
    }

    newVideoModeIndex = currentVideoModeIndex;

}

std::vector<unsigned int> Window::getCharacterInput()
{
    return frameCodePoints;
}

void Window::setTitle(const std::string& s)
{

    #ifndef COPPERY_HEADLESS
    glfwSetWindowTitle(glWindow,s.c_str());
    #endif

}

void Window::setWindowIcon(const std::vector<std::string>& fnames)
{
    #ifndef COPPERY_HEADLESS
    std::vector<GLFWimage> images;

    for (auto& fname : fnames)
    {
        size_t len;
        char* fdata = GetFileContentsCopy(fname, &len);
        if (fdata)
        {
            int n;
            int w;
            int h;
            unsigned char *data = nullptr;
            data = stbi_load_from_memory((unsigned char*)fdata, len, &w, &h, &n, 4);
            delete[] fdata;

            if (data)
            {
                GLFWimage im;
                im.width = w;
                im.height = h;
                im.pixels = data;
                images.push_back(im);
            }
        }
        else 
            Log << "Failed to load window icon " << fname << Trace(CHash("Warning"));
    }

    glfwSetWindowIcon(glWindow, images.size(), images.data());

    for (auto& im : images)
        stbi_image_free(im.pixels);
    #endif
}

Window::Window()
{
    
}

Window::~Window()
{

}
