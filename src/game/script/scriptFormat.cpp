//Angelscript object deserialization and xml reading

#include <angelscript.h>
#include <format.hpp>

#include "script.hpp"


//Class that wraps a FormatString for the scripts
class ScriptFormatString : public FormatString
{
public:
    int ref = 1;
    
    bool compileStdString(const std::string& input)
    {
        return compile(input.c_str());
    }

    void addRef()
    {
        ref++;
    }

    void release()
    {
        ref--;
        if (ref <= 0)
        {
            delete this;
        }
    }
};


//Class that wraps a FormatStringFormatter for the scripts
//Just use inheritance
class ScriptFormatStringFormatter : public FormatStringFormatter
{
public:
    
    int ref = 1;

    void addRef()
    {
        ref++;
    }

    void release()
    {
        ref--;
        if (ref <= 0)
        {
            delete this;
        }
    }
};

ScriptFormatStringFormatter* FormatterFactory()
{
    return new ScriptFormatStringFormatter();
}


ScriptFormatString* FormatStringFactory()
{
    return new ScriptFormatString();
}


void ScriptEngine::defineFormatString()
{
    int r = 0;


    r = ase->RegisterObjectType("FormatString",0, asOBJ_REF);
    assert (r >= 0);

    r = ase->RegisterObjectBehaviour("FormatString", asBEHAVE_FACTORY, "FormatString@ f()", asFUNCTION(FormatStringFactory), asCALL_CDECL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectBehaviour("FormatString", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptFormatString, addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("FormatString", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptFormatString, release), asCALL_THISCALL);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("FormatString", "bool compile(const string &in input)", asMETHOD(ScriptFormatString, compileStdString), asCALL_THISCALL);
    assert(r >= 0);


    r = ase->SetDefaultNamespace("FormatString");
    assert(r >= 0);

    r = ase->RegisterObjectType("Formatter",0, asOBJ_REF);
    assert (r >= 0);

    r = ase->RegisterObjectBehaviour("Formatter", asBEHAVE_FACTORY, "Formatter@ f()", asFUNCTION(FormatterFactory), asCALL_CDECL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("Formatter", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptFormatStringFormatter, addRef), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("Formatter", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptFormatStringFormatter, release), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("Formatter", "int format(const FormatString& fs, string &out output)", asMETHOD(ScriptFormatStringFormatter, format), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->RegisterObjectMethod("Formatter", "int formatWithParent(const FormatString& fs, const Formatter &in parent, string &out output)", asMETHOD(ScriptFormatStringFormatter, formatWithParent), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->RegisterObjectMethod("Formatter", "void add(const string &in arg)", asMETHODPR(ScriptFormatStringFormatter, add, (const std::string&), void), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->RegisterObjectMethod("Formatter", "void add(const int arg)", asMETHODPR(ScriptFormatStringFormatter, add, (const int), void), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->RegisterObjectMethod("Formatter", "void add(const double arg)", asMETHODPR(ScriptFormatStringFormatter, add, (const double), void), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->RegisterObjectMethod("Formatter", "void set(const string &in arg, size_t index)", asMETHODPR(ScriptFormatStringFormatter, set, (const std::string&, size_t), void), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->RegisterObjectMethod("Formatter", "void set(const int arg, size_t index)", asMETHODPR(ScriptFormatStringFormatter, set, (const int, size_t), void), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->RegisterObjectMethod("Formatter", "void set(const double arg, size_t index)", asMETHODPR(ScriptFormatStringFormatter, set, (const double, size_t), void), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->RegisterObjectMethod("Formatter", "void clear(size_t index)", asMETHODPR(ScriptFormatStringFormatter, clear, (size_t), void), asCALL_THISCALL);
    assert(r >= 0);
    
    r = ase->SetDefaultNamespace("");
    assert(r >= 0);
    (void)(r);

}
