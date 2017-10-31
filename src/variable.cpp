#include "variable.hpp"
#include <cassert>
#include <cstring>
#include <sstream>
#include "log.hpp"
#include "config.hpp"
#include "stringUtil.hpp"

void GameVariable::rawSetNumber(numberType n)
{
    auto prev = n_value;
    n_value= n;
    if (!checkLimits())
        n_value = prev;

}

void GameVariable::rawSetInteger(integerType i)
{
    auto prev = i_value;
    i_value = i;
    if (!checkLimits())
        i_value = prev;
}

void GameVariable::setBoolean(bool b)
{
    if (type == Number)
        rawSetNumber(b ? 1.0 : 0);
    else if (type == Integer)
        rawSetInteger(b ? 1 : 0);
}

void GameVariable::setNumber(numberType d)
{
    if (type == Number)
        rawSetNumber(d);
    else if (type == Integer)
        rawSetInteger((integerType)d);
}

void GameVariable::setInteger(integerType i)
{
    if (type == Number)
        rawSetNumber((numberType)i);
    else if (type == Integer)
        rawSetInteger(i);
}

void GameVariable::setString(const char* cstr)
{
    if (type == String && cstr)
    {
        auto prev = std::move(s_value);
        s_value = cstr;
        if (!checkLimits())
            s_value = std::move(prev);

    }
}

void GameVariable::setString(const std::string& str)
{
    setString(str.c_str());
}


std::string GameVariable::toString() const
{
    if (type == String)
        return s_value;
    if (type == Number)
        return std::to_string(n_value);
    if (type == Integer)
        return std::to_string(i_value);
    assert(false);
    return "";
}


template <typename T>
bool parse(const char* str, T& d)
{
    std::stringstream ss(str);
    ss >> d;
    if (ss.fail())
        return false;
    ss >> std::ws;
    if (!ss.eof())
        return false;
    return true;
}


bool GameVariable::parseFrom(const char* c)
{
    if (c == nullptr)
        return false;


    if (type == String)
    {
        std::string temp(c);
        StringUtil::Trim(temp);	
        setString(temp);
        return true;
    }
    else if (type == Number || type == Integer)
    {

        integerType i;
        
        if (!parse(c, i))
        {
            numberType d;
            if (!parse(c, d))
            {
                std::string temp;

                std::stringstream ss(c);
                ss >> temp;
                if (ss.fail())
                    return false;

                if (temp == "false")
                {
                    setBoolean(false);
                    return true;
                }
                else if (temp == "true")
                {
                    setBoolean(true);
                    return true;
                }
                

                return false;
            }

            setNumber(d);
            return true;
        }

        setInteger(i);
        return true;
    }

    assert(false);
    return false;
}

void GameVariable::hideFromTrace()
{
    if (!hiddenFromTrace)
    {
        if (varName.size() == 0)
            Log << "[Anonymous variable]";
        else
            Log << varName;
        Log << " tracing disabled";
        Log << Trace(CHash("GameVarTrace"));

    }
    hiddenFromTrace = true;
}

void GameVariable::makeConstant()
{
    if (!constant)
    {
        Log << "Constant ";
        if (varName.size() == 0)
            Log << "[Anonymous variable]";
        else
            Log << varName;
        Log << " <- ";

        Log << toString();
        Log << Trace(CHash("GameVarTrace"));
    }
    constant = true;
}

bool GameVariable::parseFrom(const std::string& str)
{
    return parseFrom(str.c_str());
}


bool GameVariable::checkLimits()
{
    if (constant)
        return false;

    if (bounded)
    {
        if (type == Integer)
        {
            if (i_value > i_max)
                i_value = i_max;
            if (i_value < i_min)
                i_value = i_min;
        }
        else if (type == Number)
        {
            if (n_value > n_max)
                n_value = n_max;
            if (n_value < n_min)
                n_value = n_min;
        }
    }
    bool allowedChange = true;
    if (updateHandler)
    {
        allowedChange = updateHandler(*this);
    }

    if (!hiddenFromTrace)
    {
        if (!allowedChange)
            Log << "(denied) ";

        if (varName.size() == 0)
            Log << "[Anonymous variable]";
        else
            Log << varName;
        

        Log << " <- ";
        Log << toString();
        Log << Trace(CHash("GameVarTrace"));
        
    }
    return allowedChange;
}

void GameVariableManager::sanitizeName(std::string& name)
{
    StringUtil::Trim(name);	
}

GameVariable* GameVariableManager::addVariable(const std::string& name)
{
    

    std::string namecpy = name;
    sanitizeName(namecpy);

    gameVars.push_back({namecpy, std::make_unique<GameVariable>()});

    GameVariable* var = (gameVars.end() - 1)->second.get();

    var->varIndex = gameVars.size() - 1;
    var->varName = namecpy;



    if (namecpy.size() == 0)
    {
        Log << "Invalid GameVariable name '" << name << "'" << Trace(CHash("Warning"));
    }
    else
    {
        Hash hash = Hash(namecpy);
        auto it = gameVarsByHash.find(hash);
        if (it != gameVarsByHash.end())
            Log << "Name collision on GameVariable '" << namecpy << "'" << Trace(CHash("Warning"));
        else
        {
            Log << "Initialized Variable " << namecpy << Trace(CHash("GameVarTrace"));
            gameVarsByHash[hash] = gameVars.size() - 1;
            gameVarsByName[namecpy] = gameVars.size() - 1;
        }
    }
    return var;
}
GameVariable* GameVariableManager::makeNumber(const std::string& name, GameVariable::NumberType def, GameVariable::UpdateHandlerType handler)
{
    auto* var = addVariable(name);
    var->type = GameVariable::Number;
    var->updateHandler = handler;

    var->setNumber(def);
    return var;
}

GameVariable* GameVariableManager::makeInteger(const std::string& name, GameVariable::IntegerType def, GameVariable::UpdateHandlerType handler)
{
    auto* var = addVariable(name);
    var->type = GameVariable::Integer;
    var->updateHandler = handler;

    var->setInteger(def);
    return var;
}

#define MINIMUM(a,b) ((a) < (b) ? (a) : (b))
#define MAXIMUM(a,b) ((a) > (b) ? (a) : (b))

GameVariable* GameVariableManager::makeNumberLimits(const std::string& name, GameVariable::NumberType min, GameVariable::NumberType max, GameVariable::NumberType def, GameVariable::UpdateHandlerType handler)
{
    if (min > max)
    {
        auto b = max;
        max = min;
        min = b;
    }
    def = MAXIMUM(MINIMUM(def, max), min);

    auto* var = addVariable(name);
    var->type = GameVariable::Number;
    var->n_min = min;
    var->n_max = max;
    var->bounded = true;
    var->updateHandler = handler;

    var->setNumber(def);

    return var;
}
GameVariable* GameVariableManager::makeIntegerLimits(const std::string& name, GameVariable::NumberType min, GameVariable::NumberType max, GameVariable::IntegerType def, GameVariable::UpdateHandlerType handler)
{
    if (min > max)
    {
        auto b = max;
        max = min;
        min = b;
    }

    def = MAXIMUM(MINIMUM(def, max), min);

    auto* var = addVariable(name);
    var->type = GameVariable::Integer;
    var->i_min = min;
    var->i_max = max;
    var->bounded = true;
    var->updateHandler = handler;

    var->setInteger(def);
    return var;
}

GameVariable* GameVariableManager::makeString(const std::string& name, const char* def , GameVariable::UpdateHandlerType handler)
{
    auto* var = addVariable(name);
    var->updateHandler = handler;
    var->type = GameVariable::String;
    if (def)
        var->setString(def);
    else
        var->setString("");

    return var;
}

GameVariable* GameVariableManager::makeString(const std::string& name, const std::string& def, GameVariable::UpdateHandlerType handler)
{
    return makeString(name, def.c_str(), handler);
}

bool GameVariableManager::getNumber(Hash h, GameVariable::NumberType & out) const
{
    auto * ptr = get(h);
    if (!ptr)
        return false;
    if (ptr->type == GameVariable::Integer || ptr->type == GameVariable::Number)
    {
        out = ptr->getNumber();
        return true;
    }
    return false;
}

bool GameVariableManager::getInteger(Hash h, GameVariable::IntegerType & out) const
{
    auto * ptr = get(h);
    if (!ptr)
        return false;

    if (ptr->type == GameVariable::Integer || ptr->type == GameVariable::Number)
    {
        out = ptr->getInteger();
        return true;
    }
    return false;
}


bool GameVariableManager::getString(Hash h, std::string & out) const
{
    auto * ptr = get(h);
    if (!ptr)
        return false;

    if (ptr->type == GameVariable::String)
    {
        out = ptr->getString();
        return true;
    }
    return false;
}


GameVariable::NumberType GameVariableManager::getNumberDefault(Hash h, GameVariable::NumberType defaultVal) const
{
    getNumber(h, defaultVal);
    return defaultVal;
}

GameVariable::IntegerType GameVariableManager::getIntegerDefault(Hash h, GameVariable::IntegerType defaultVal) const
{
    getInteger(h, defaultVal);
    return defaultVal;
}


std::string GameVariableManager::getStringDefault(Hash h, const std::string& defaultVal) const
{
    std::string returnval;
    if (!getString(h, returnval))
        returnval = defaultVal;
    return returnval;
}

GameVariable* GameVariableManager::get(Hash h) const
{
    auto it = gameVarsByHash.find(h);
    if (it == gameVarsByHash.end())
        return nullptr;
    return gameVars[it->second].second.get();

}

GameVariable* GameVariableManager::getByString(const std::string& s) const
{
    auto it = gameVarsByName.find(s);
    if (it == gameVarsByName.end())
        return nullptr;
    return gameVars[it->second].second.get();
}

GameVariableManager::GameVariableManager()
{
    config = std::make_unique<ConfigCollection>();
}


void GameVariableManager::loadFromConfig(const std::vector<GameVariable*>& variables, const char* group)
{
    if (group == nullptr)
        throw std::runtime_error("loadFromConfig group == nullptr");

    for (auto* var : variables)
    {
        
        std::string buffer = "";
        if (var->varIndex < 0 || static_cast<size_t>(var->varIndex) >= gameVars.size())
            continue;
        auto& name = gameVars[var->varIndex].first;

        if (config->getConfiguration(group, name.c_str(), buffer))
        {
            if (!var->parseFrom(buffer))
                Log << "Invalid configuration for variable '" << name << "' in configuration group '" << group << "'" <<  Trace(CHash("Warning"));
                
        }
    }
}

size_t GameVariableManager::getCount()
{
    return gameVars.size();
}

void GameVariableManager::getPrefixedMatch(const char* name, std::vector<const char*>& out)
{
    size_t namelen = std::strlen(name);
    auto i = gameVarsByName.lower_bound(name);
    while (i != gameVarsByName.end())
    {
        if (i->first.compare(0, namelen, name) == 0)
            out.push_back(i->first.c_str());
        else
            break;
        ++i;
    }
}

void GameVariableManager::loadAllFromConfigGroups(char separator)
{
    for (auto& p : gameVars)
    {
        auto& var = p.second;
        std::string group = p.first;
        std::string after = StringUtil::SplitFirst(group, separator);
        if (after.size() == 0)
        {
            //If the separater was not found, the variable is a GLOBAL variable
            //they are never automatically saved
            continue;
        }
        std::string buffer;
        if (config->getConfiguration(group.c_str(), after.c_str(), buffer))
        {
            if (!var->parseFrom(buffer))
                Log << "Invalid configuration for variable '" << p.first << "' in configuration group '" << group << "'" <<  Trace(CHash("Warning"));
        }
    }
}

GameVariableManager::~GameVariableManager()
{

}
