#include "script.hpp"
#include "variable.hpp"
#include "game/game.hpp"
#include "regHelper.hpp"
#include <angelscript.h>
#include <cassert>

void ScriptEngine::defineGameVar()
{
    int r;

    GameVariableManager* gvarMan = nullptr;
     if (!compilerOnly)
    {
         gvarMan = engine->getVariableManager();
         
    }

    r = ase->RegisterObjectType("GameVar", 0, asOBJ_REF | asOBJ_NOCOUNT);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("GameVar", "void set(int)", asMETHOD(GameVariable, setInteger), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("GameVar", "void set(double)", asMETHOD(GameVariable, setNumber), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("GameVar", "void set(string)", asMETHODPR(GameVariable, setString, (const std::string&), void), asCALL_THISCALL);
    assert( r >= 0 );


    r = ase->RegisterObjectMethod("GameVar", "double getDouble()", asMETHOD(GameVariable, getNumber), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("GameVar", "int getInteger()", asMETHOD(GameVariable, getInteger), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("GameVar", "string getString()", asMETHOD(GameVariable, getString), asCALL_THISCALL);
    assert( r >= 0 );


    r = ase->SetDefaultNamespace("GameVar");
    assert(r >= 0);

    r = registerGlobalFunctionAux(this,"GameVar@ GetVar(hash_t name)", asMETHOD(GameVariableManager, get), asCALL_THISCALL_ASGLOBAL, gvarMan);
    assert( r >= 0 );

    r = registerGlobalFunctionAux(this,"GameVar@ GetVar(const string &in name)", asMETHOD(GameVariableManager, getByString), asCALL_THISCALL_ASGLOBAL, gvarMan);
    assert( r >= 0 );

    r = registerGlobalFunctionAux(this,"double GetDouble(hash_t name, double)", asMETHOD(GameVariableManager, getNumberDefault), asCALL_THISCALL_ASGLOBAL, gvarMan);
    assert( r >= 0 );

    r = registerGlobalFunctionAux(this,"int GetInteger(hash_t name, int)", asMETHOD(GameVariableManager, getNumberDefault), asCALL_THISCALL_ASGLOBAL, gvarMan);
    assert( r >= 0 );

    r = registerGlobalFunctionAux(this,"string GetString(hash_t name, const string &in)", asMETHOD(GameVariableManager, getNumberDefault), asCALL_THISCALL_ASGLOBAL, gvarMan);
    assert( r >= 0 );

    r = ase->SetDefaultNamespace("");
    assert(r >= 0);
    (void)(r);
}
