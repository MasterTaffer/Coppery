#include <luawrap.hpp>
#include "variable.hpp"
#include "gameVarInterface.hpp"

static GameVariable* newVarNumberNoLimits(GameVariableManager* varman, std::string name, GameVariable::NumberType def)
{
    return varman->makeNumber(name, def);
}

static GameVariable* newVarIntegerNoLimits(GameVariableManager* varman, std::string name, GameVariable::IntegerType def)
{
    return varman->makeInteger(name, def);
}


static GameVariable* newVarNumber(GameVariableManager* varman, std::string name, GameVariable::NumberType min, GameVariable::NumberType max, GameVariable::NumberType def, GameVariable::UpdateHandlerType handler)
{
    return varman->makeNumberLimits(name, min, max, def, handler);
}

static GameVariable* newVarInteger(GameVariableManager* varman, std::string name, GameVariable::IntegerType min, GameVariable::NumberType max, GameVariable::IntegerType def, GameVariable::UpdateHandlerType handler)
{
    return varman->makeIntegerLimits(name, min, max, def, handler);
}

static GameVariable* newVarNumberNoHandle(GameVariableManager* varman, std::string name, GameVariable::NumberType min, GameVariable::NumberType max, GameVariable::NumberType def)
{
    return varman->makeNumberLimits(name, min, max, def);
}

static GameVariable* newVarIntegerNoHandle(GameVariableManager* varman, std::string name, GameVariable::IntegerType min, GameVariable::NumberType max, GameVariable::IntegerType def)
{
    return varman->makeIntegerLimits(name, min, max, def);
}


static GameVariable* getVar(GameVariableManager* varman, std::string name)
{
    return varman->getByString(name);
}

static int getVarType(GameVariableManager* varman, GameVariable* var)
{
    if (var)
        return var->getType(); 
    return -1;
}


static void setInteger(GameVariableManager* varman,  GameVariable* gv, GameVariable::IntegerType number)
{
    if (gv)
        gv->setInteger(number);
}

static void setNumber(GameVariableManager* varman,  GameVariable* gv, GameVariable::NumberType number)
{
    if (gv)
        gv->setNumber(number);

}

static GameVariable::NumberType getNumber(GameVariableManager* varman, GameVariable* gv)
{
    if (gv)
        return gv->getNumber();

    return 0;
}

static GameVariable::IntegerType getInteger(GameVariableManager* varman, GameVariable* gv)
{
    if (gv)
        return gv->getInteger();

    return 0;
}


static GameVariable* newConstantNumber(GameVariableManager* varman, std::string name, GameVariable::NumberType def)
{
    auto p = varman->makeNumber(name, def);
    p->makeConstant();
    return p;
}

static GameVariable* newConstantInteger(GameVariableManager* varman, std::string name, GameVariable::IntegerType def)
{
    auto p = varman->makeInteger(name, def);
    p->makeConstant();
    return p;
}

static int getVarCount(GameVariableManager* varman)
{
    
    return varman->getCount();
}
static std::vector<const char*> getPrefixedMatch(GameVariableManager* varman, const char* str)
{
    std::vector<const char*> out;
    varman->getPrefixedMatch(str, out);
    return out;
}

luaL_Reg gameVar_functions[] =
{
    {"GetCount", LuaClosureWrap(getVarCount, 1)},
    {"GetPrefixedMatch", LuaClosureWrap(getPrefixedMatch, 1)},
    
    
    {"NewNumberConstant", LuaClosureWrap(newConstantNumber, 1)},
    {"NewIntegerConstant", LuaClosureWrap(newConstantInteger, 1)},
    
    {"NewNumberHandle", LuaClosureWrap(newVarNumber, 1)},
    {"NewNumberLimits", LuaClosureWrap(newVarNumberNoHandle, 1)},
    {"NewNumber", LuaClosureWrap(newVarNumberNoLimits, 1)},
    
    
    {"GetNumber", LuaClosureWrap(getNumber, 1)},
    {"SetNumber", LuaClosureWrap(setNumber, 1)},

    {"NewIntegerHandle", LuaClosureWrap(newVarInteger, 1)},
    {"NewIntegerLimits", LuaClosureWrap(newVarIntegerNoHandle, 1)},
    {"NewInteger", LuaClosureWrap(newVarIntegerNoLimits, 1)},

    {"GetInteger", LuaClosureWrap(getInteger, 1)},
    {"SetInteger", LuaClosureWrap(setInteger, 1)},


    {"GetType", LuaClosureWrap(getVarType, 1)},

    {"Get", LuaClosureWrap(getVar, 1)},
    {0,0}
};
