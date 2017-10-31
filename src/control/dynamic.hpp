#pragma once
#include <vector>
#include <string>

struct DynamicLibraryEntry;
class DynamicManager
{
    std::vector<DynamicLibraryEntry*> loadedLibraries;
    void* getVoidSymbol(DynamicLibraryEntry*, const char*);
public:
    DynamicLibraryEntry* loadLibrary(std::string c);
    
    template <typename T>
    T getSymbol(DynamicLibraryEntry* d, const char* str)
    {
        return (T)getVoidSymbol(d,str);
    };
    
    ~DynamicManager();
};

