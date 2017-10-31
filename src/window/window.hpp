
#pragma once
#include "game/engineDefs.hpp"

#include "hash.hpp"
#include "log.hpp"
#include "color.hpp"
#include "config.hpp"
#include "keys.hpp"

#include <unordered_map>
#include <set>
#include <list>
#include <random>
#include <vector>

class GameVariableManager;

struct GLFWwindow;
class Window;
class InputCascade;


class KeyConfig;

/*! \brief System for managing the display window and associated events.
*/
class Window
{
    GLFWwindow* glWindow = nullptr;
    Vector2i dimensions = {0,0};
    Vector2d mousePos = {0.0,0.0};
    bool closing = false;

    std::vector<KeyInputStyle> keyInputs;
    std::vector<MouseButtonInputStyle> mouseButtonInputs;
    std::vector<JoyButtonInputStyle> joyButtonInputs;
    std::vector<JoyAxisInputStyle> joyAxisInputs;

    std::vector<std::pair<int, float>> inputBuffer;

    std::unique_ptr<InputCascade> inputCascade;

    void updateInputs();

    bool anyKeyPressed = false;
    void updateVideoModes();

    std::vector<Vector2i> videoModes;
    int currentVideoModeIndex = 0;
    Vector2i videoModeAfterRestart = {0,0};
    bool fullscreen = false;
    bool fullscreenAfterRestart = false;

    int swapInterval = 0;

    Vector2i outputDimensions;
    KeyConfig* keyConfig;

    float joystickDeadZone = 0.0f;
    std::vector<unsigned int> frameCodePoints;
    int newVideoModeIndex = 0; 
public:

    //! Set the new video mode index after restart
    void setNewVideoModeIndex(int idx)
    {
        if (idx < 0 || (unsigned int)idx >= videoModes.size())
            return;
        setNewVideoMode(videoModes[idx]);
        newVideoModeIndex = idx;
    }

    //! Set the new video mode after restart
    void setNewVideoMode(Vector2i d);

    //! Get if the window is in fullscreen
    bool getIsFullScreen() {return fullscreen;}

    //! Set the fullscreen setting after restart
    void setNewFullScreen(bool b) {fullscreenAfterRestart = b;}

    //! Get the fullscreen setting after restart
    bool getNewFullScreen() {return fullscreenAfterRestart;}

    //! Get current video mode index
    int getCurrentVideoModeIndex() {return currentVideoModeIndex;}

    //! Get the new video mode index after restart
    int getNewVideoModeIndex() {return newVideoModeIndex;}

    //! Get suggested video modes
    std::vector<Vector2i>& getVideoModes();

    //! Initiate the window closing sequence
    void close() {closing = true;}

    //! Get if any key was pressed during last step
    bool getAnyKeyPressed() { return anyKeyPressed; }

    //! Get applcation runtime in seconds
    double getTime();

    //! Get mouse position within the window
    Vector2d getMousePosition()
    {
        return mousePos;
    };

    //! Get reference to InputCascade
    InputCascade* getInputCascade()
    {
        return inputCascade.get();
    }

    //! Initializes Window system
    void init(GameVariableManager* var);

    //! Deinitializes Window system
    void deInit();

    /*! \brief Steps the Window system
        
        Performs event polling and swaps buffers.
    */

    void update();

    /*! \brief Gets the current size of the window FrameBuffer

        Value returned by this may differ from getWindowDimensions, as this
        function returns the current size of the framebuffer. Certain window
        managers provide a different window framebuffer size for a split
        second after window creation.
    */
    Vector2i getOutputDimensions() {return outputDimensions;}

    //! Gets official, requested size of the window
    Vector2i getWindowDimensions() {return dimensions;};

    //! Gets the current closing status of Window
    bool isClosing() {return closing;};

    //! Sets the title of the window
    void setTitle(const std::string&);

    //! Load the window icon from file(s)
    void setWindowIcon(const std::vector<std::string>& fnames);

    //! Get UTF-32 codepoints input during last input step
    std::vector<unsigned int> getCharacterInput();

    //! Constructor
    Window();

    //! Destructor
    ~Window();
};
