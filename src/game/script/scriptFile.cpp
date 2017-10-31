#include "script.hpp"
#include "log.hpp"

#include <cstddef>
#include <angelscript.h>
#include <cassert>
#include <fstream>

class ScriptFile
{
    bool openForWriting = false;
    std::fstream file;
public:
    ScriptFile(std::string f, bool write = false)
    {
        openForWriting = write;
        if (write)
            file.open(f,std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
        else
            file.open(f,std::ios_base::in | std::ios_base::binary);
    }

    bool getEOF()
    {
        if (openForWriting)
            return false;
        return (file.peek() == EOF);
    }

    template<typename T>
    void write(T i)
    {
        if (openForWriting)
            file.write(reinterpret_cast<char*>(&i),sizeof(i));
    }

    template<typename T>
    T read()
    {
        if (openForWriting)
            return 0;
        T i;
        file.read(reinterpret_cast<char*>(&i),sizeof(i));
        return i;
    }

};


void ScriptEngine::defineFile()
{
    int r = 0;
    r = ase->RegisterObjectType("File",0, asOBJ_REF | asOBJ_NOCOUNT);
    assert (r >= 0);

    r = ase->RegisterObjectMethod("File","bool isEOF()", asMETHOD(ScriptFile,getEOF), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod("File","int32 readInt32()", asMETHODPR(ScriptFile,read<int32_t>, (void), int32_t), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod("File","void writeInt32(int32)", asMETHODPR(ScriptFile,write<int32_t>, (int32_t), void), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod("File","int8 readInt8()", asMETHODPR(ScriptFile,read<int8_t>, (void), int8_t), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod("File","void writeInt8(int8)", asMETHODPR(ScriptFile,write<int8_t>, (int8_t), void), asCALL_THISCALL);
    assert (r >= 0);
    (void)(r);

}


