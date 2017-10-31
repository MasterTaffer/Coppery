#pragma once



/*! \brief Class for single Input axle
 * 
 * Provides an interface for single input axle, either in digital or analog
 * or both. The digital state may be 0, 1 or -1, depending on the Input.
 * Typically on keyboard keys the values are 1 if the key is down and 0 if
 * not. -1 is a state possible with analog axles such as controller analog
 * sticks. 
 * 
 * The analog state typically varies from -1.0 to 1.0 where 0.0 is the
 * "neutral". 
 * 
 * The pressed and released states specifies the difference in digital state:
 * If the Input changed from 0 to 1 (e.g keyboard key was pressed) the pressed
 * state will be 1, and if the state changes from 1 to 0 (the key was released)
 * the released state will be 1. If the digital state stays constant, the
 * pressed and released states will be 0 both.
 * 
 * It is also possible for pressed and released states to be -1, if the Input
 * change was towards the negative direction, e.g an analog stick was struck 
 * downwards from state 0 to -1.
 */
class Input
{
    //TODO figure out a solution for sub step input differences
    //  if an input was pressed and quickly released within the step
    
    enum Type
    {
        Analog,
        Digital
    };
    Type type = Type::Digital;
    int down = 0;
    int nDown = 0;
    int released = 0;
    int pressed = 0;
    float input = 0.0f;
    float nInput = 0.0f;
public:

    //! Gets the digital just pressed state 
    int getPressed() const
    {
        return pressed;
    };

    //! Gets the digital state
    int getState() const
    {
        return down;
    };

    //! Gets the digital just released state
    int getReleased() const
    {
        return released;
    };

    //! Gets the analog state
    float getValue() const
    {
        return input;
    }

    friend class Window;
    friend class InputLayer;
    friend class InputCascade;
private:
    int analogToDigital(float d)
    {
        const float analogTrigger = 0.9f;
        if (d > analogTrigger)
            return 1;
        if (d < -analogTrigger)
            return -1;
        return 0;
    }

    void updateAnalog(float newInput)
    {
        nInput += newInput;
        nDown += analogToDigital(input);
    }

    void update()
    {
        released = 0;
        pressed = 0;

        if (nDown > 1) nDown = 1;
        if (nDown < -1) nDown = -1;

        if (down != nDown)
        {
            if (down)
                released = down;
            else
                pressed = nDown;
        }
        down = nDown;
        input = nInput;
        nInput = 0.0f;
        nDown = 0;
    }
};
