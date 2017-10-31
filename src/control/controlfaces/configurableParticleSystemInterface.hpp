#pragma once
#include <lua.hpp>

extern luaL_Reg configurableParticleSystem_functions[3];

void RegisterConfigurableParticleSystem(Graphics* , lua_State* );