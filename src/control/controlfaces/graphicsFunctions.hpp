#pragma once
#include <luawrap.hpp>
class Graphics;

extern luaL_Reg graphicsFunctions_functions[25];

void RegisterGraphicsFunctions(Graphics* , lua_State* );