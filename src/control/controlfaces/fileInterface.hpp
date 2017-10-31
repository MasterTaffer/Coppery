#pragma once
#include <luawrap.hpp>

extern luaL_Reg file_functions[4];

void RegisterFileFunctions(lua_State*);
