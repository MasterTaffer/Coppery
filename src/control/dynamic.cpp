#include "dynamic.hpp"

#ifdef _WIN32

#include <windows.h>
typedef HMODULE DynamicLibrary;
#define DynamicLoadLibrary(x) LoadLibrary(x)
#define DynamicGetSymbol(x,sym) ((void*) GetProcAddress(x,sym))
#define DynamicUnloadLibrary(x) FreeLibrary(x)
#define DynamicLibraryPostfix ".dll"
#define DynamicLibraryPrefix ".\\"

#else

#include <dlfcn.h>
typedef void* DynamicLibrary;
#define DynamicLoadLibrary(x) dlopen(x, RTLD_LAZY)
#define DynamicGetSymbol(x,sym) dlsym(x,sym)
#define DynamicUnloadLibrary(x) dlclose(x)
#define DynamicLibraryPostfix ".so"
#define DynamicLibraryPrefix "./"

#endif


struct DynamicLibraryEntry
{
    DynamicLibrary library;
};

void* DynamicManager::getVoidSymbol(DynamicLibraryEntry* dl, const char* str)
{
    return DynamicGetSymbol(dl->library, str);
}

DynamicLibraryEntry* DynamicManager::loadLibrary(std::string str)
{
    std::string name = std::string(DynamicLibraryPrefix) + str + DynamicLibraryPostfix;
    DynamicLibrary dl = DynamicLoadLibrary(name.c_str());
    if (!dl)
    {
        return nullptr;
    }
    DynamicLibraryEntry* dle = new DynamicLibraryEntry;
    dle->library = dl;
    loadedLibraries.push_back(dle);
    return dle;
}

DynamicManager::~DynamicManager()
{
    for (auto a : loadedLibraries)
    {
        DynamicUnloadLibrary(a->library);
        delete a;
    }
    loadedLibraries.clear();
}

