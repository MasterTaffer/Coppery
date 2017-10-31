#pragma once
#define DynamicSharedCurrentVersion 104

#ifdef _WIN32
#define DynamicExportFunction __declspec(dllexport)
#else
#define DynamicExportFunction
#endif

#ifndef IS_MAIN_APPLICATION
typedef void MainApplication;
#else
class Control;
typedef Control MainApplication;
#endif

class asIScriptEngine;
struct lua_State;

struct RegParticleSystemParameters;


typedef void(*ASInitHandler)(asIScriptEngine*);
typedef void(*LuaInitHandler)(lua_State*);
typedef void(*InitHandler)();
typedef void(*DeInitHandler)();

struct DynamicSharedInitializationData
{
    int version;
    void(*message)(MainApplication*, const char*);
    void(*registerASInitHandler)(MainApplication*, ASInitHandler);
    void(*registerLuaInitHandler)(MainApplication*, LuaInitHandler);

    void(*registerInitHandler)(MainApplication*, InitHandler);
    void(*registerDeInitHandler)(MainApplication*, DeInitHandler);
    bool(*registerParticleSystem)(MainApplication*, RegParticleSystemParameters*);
    int(*getUniformLocation)(MainApplication*, const char*, const char*);
};
