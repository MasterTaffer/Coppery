#pragma once
#include <string>
class InputStyle
{
public:
    //! Input name to describe the input
    std::string inputName;
    int inputIndex = -1;
};

//! Input style for keyboard
class KeyInputStyle : public InputStyle
{
public:
    int key = 0;
    bool reversed = false;
    bool repeat = false;
};


//! Input style for mouse buttons
class MouseButtonInputStyle : public InputStyle
{
public:
    int key = 0;
    bool reversed = false;
};

//! Input style for controller buttons
class JoyButtonInputStyle : public InputStyle
{
public:
    int joy = 0;
    int button = 0; 
    bool reversed = false;
};

//! Input style for controller axes
class JoyAxisInputStyle : public InputStyle
{
public:

    int joy = 0;
    int axis = 0;

    bool reversed = false;
};
