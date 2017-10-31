#pragma once
#include "script.hpp"
#include <angelscript.h>

/*! \brief A helper function used to register AS functionality
 * 
 * This function exists as a helper to register AngelScript global functions.
 * Many global functions are direct call to class methods, and in the case of
 * compiler only mode, these class objects may not exists. In these cases the
 * aux pointer (the object) will be nullptr. AngelScript however does not
 * approve of nullptr aux pointer in asTHISCALL calling conventions,
 * returning an error code and failing.
 * 
 * In case of compiler only mode one should pass a null function pointer
 * instead under the asFUNCTION and asCALL_CDECL calling convention. This
 * function provides a convenient wrapper for this.
 * 
 * The parameters for this function except for the first one are the same
 * as for the angelscript asIScriptEngine::RegisterGlobalFunction.
 * 
 * \param se ScriptEngine instance, must not be nullptr
 * \param declaration the declaration parameter as in RegisterGlobalFunction
 * \param funcPointer the funcPointer parameter as in RegisterGlobalFunction
 * \param callConv the callConv parameter as in RegisterGlobalFunction
 * \param aux the aux parameter as in RegisterGlobalFunction
 * \return AngelScript RegisterGlobalFunction return code
 */

static int registerGlobalFunctionAux(ScriptEngine* se, const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void* aux = nullptr)
{
    asIScriptEngine* ase = se->getScriptEngine();
    if (se->isCompilerOnly())
    {
        return ase->RegisterGlobalFunction(declaration, asFUNCTION(nullptr), asCALL_CDECL);
    }
    else
        return ase->RegisterGlobalFunction(declaration, funcPointer, callConv, aux);
}
