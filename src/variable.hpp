#pragma once
#include <string>
#include <memory>
#include <vector>
#include <cassert>

#include <map>
#include <unordered_map>
#include <limits>
#include "hash.hpp"
#include "control/crossFunctional.hpp"



#ifndef CROSSFUNCTION_NO_LUA
class GameVariable;
namespace LuaFunctionWrapper
{
    template <>
    struct ReturnHandler<GameVariable>;
}
#endif

/*! \brief Dynamic typed variable

    See possible variable types at GameVariableType. Integer and Number values
    are implicitly casted from each other if requested.
*/
class GameVariable
{
public:


    //! Possible variable types
    enum GameVariableType
    {
        Integer,
        Number,
        String
    };

    //! Update handler callback type
    typedef CrossFunction<bool, const GameVariable&> UpdateHandlerType;

    //! Integer variable type
    typedef int IntegerType;

    //! Number variable type
    typedef double NumberType;

    //! Maximum numeric value representable by IntegerType
    constexpr static IntegerType IntegerMax = std::numeric_limits<GameVariable::IntegerType>::max();
    //! Minimum numeric value representable by IntegerType
    constexpr static IntegerType IntegerMin = std::numeric_limits<GameVariable::IntegerType>::min();

    //! Maximum numeric value representable by NumberType
    constexpr static NumberType NumberMax = std::numeric_limits<GameVariable::NumberType>::max();
    //! Minimum numeric value representable by NumberType
    constexpr static NumberType NumberMin = std::numeric_limits<GameVariable::NumberType>::min();


protected:
    typedef IntegerType integerType;
    typedef NumberType numberType;
    bool bounded = false;

    double n_min = 0.0;
    double n_max = 1.0;

    int i_min = 0;
    int i_max = 1;


    numberType n_value;
    integerType i_value;
    
    GameVariableType type;
    std::string s_value;

    bool constant = false;
    bool hiddenFromTrace = false;
    bool checkLimits();
    UpdateHandlerType updateHandler;

    void rawSetNumber(numberType);
    void rawSetInteger(integerType);

    int varIndex = -1;
    std::string varName;

public:
    //! Constructor
    GameVariable() {};

    //! Copy constructor
    GameVariable(const GameVariable& other)
    {
        type = other.type;

        bounded = other.bounded;

        i_min = other.i_min;
        i_max = other.i_max;

        n_min = other.n_min;
        n_max = other.n_max;

        n_value = other.n_value;
        i_value = other.i_value;

        s_value = other.s_value;
    }

    //! Comparison operator overload
    bool operator == (const GameVariable& other) const
    {
        if (type != other.type)
            return false;

        if (type == Number)
            return n_value == other.n_value;
        else if (type == Integer)
            return i_value == other.i_value;
        else if (type == String)
            return s_value == other.s_value;

        assert(false);
    }

    //! Get variable type
    GameVariableType getType() const
    {
        return type;
    } 

    //! Get number value
    numberType getNumber() const
    {
        if (type == Number)
            return n_value;
        else if (type == Integer)
            return i_value;
        return 0;
    }
    
    //! Get integer value
    integerType getInteger() const
    {
        if (type == Number)
            return n_value;
        else if (type == Integer)
            return i_value;
        return 0;
    }

    //! Get string value
    const char* getString() const
    {
        return s_value.c_str();
    }

    //! Overwrite update callback
    void replaceUpdateHandler(UpdateHandlerType uh)
    {
        updateHandler = uh;
    }

    //! Set number value
    void setNumber(numberType d);

    //! Set integer value
    void setInteger(integerType i);

    //! Set boolean value
    void setBoolean(bool b);

    //! Set string value
    void setString(const char*);

    //! Set string value
    void setString(const std::string&);

    //! Make variable constant
    void makeConstant();

    //! Hide variable from GameVarTrace
    void hideFromTrace();

    //! Convert variable to string
    std::string toString() const;

    //! Parse variable from string
    bool parseFrom(const char*);

    //! Parse variable from string
    bool parseFrom(const std::string&);

    friend class GameVariableManager;
};



#ifndef CROSSFUNCTION_NO_LUA

namespace LuaFunctionWrapper
{
    template <>
    struct ReturnHandler<GameVariable>
    {
        static int get(lua_State* lua, const GameVariable& v)
        {
            switch (v.getType())
            {
                case GameVariable::Number:
                    lua_pushnumber(lua, v.getNumber());
                    return 1;
                case GameVariable::Integer:
                    lua_pushinteger(lua, v.getInteger());
                    return 1;
                case GameVariable::String:
                    lua_pushstring(lua, v.getString());
                    return 1;
            }
            lua_pushinteger(lua, 0);
            return 1;
        }
    };
}
#endif

class ConfigCollection;

//! Collection of typed variables used for storing engine wide data
class GameVariableManager
{
    std::map<std::string, size_t> gameVarsByName;
    std::unordered_map<Hash, size_t> gameVarsByHash;


    std::vector<std::pair<std::string, std::unique_ptr<GameVariable>>> gameVars;
    void sanitizeName(std::string& name);
    GameVariable* addVariable(const std::string& name);

    std::unique_ptr<ConfigCollection> config;

public:
    //! Construct a new number variable 
    GameVariable* makeNumber(const std::string& name, GameVariable::NumberType def = 0, GameVariable::UpdateHandlerType handler = GameVariable::UpdateHandlerType());
    //! Construct a new integer variable
    GameVariable* makeInteger(const std::string& name, GameVariable::IntegerType def = 0, GameVariable::UpdateHandlerType handler = GameVariable::UpdateHandlerType());
    //! Construct a new number variable with limits
    GameVariable* makeNumberLimits(const std::string& name, GameVariable::NumberType min, GameVariable::NumberType max, GameVariable::NumberType def = 0.0, GameVariable::UpdateHandlerType handler = GameVariable::UpdateHandlerType());
    //! Construct a new integer variable with limits
    GameVariable* makeIntegerLimits(const std::string& name, GameVariable::NumberType min, GameVariable::NumberType max, GameVariable::IntegerType def = 0, GameVariable::UpdateHandlerType handler = GameVariable::UpdateHandlerType());

    //! Construct a new string variable
    GameVariable* makeString(const std::string& name, const char* def = nullptr, GameVariable::UpdateHandlerType handler = GameVariable::UpdateHandlerType());

    //! Construct a new string variable
    GameVariable* makeString(const std::string& name, const std::string& def, GameVariable::UpdateHandlerType handler = GameVariable::UpdateHandlerType());

    //! Get variable reference by name
    GameVariable* getByString(const std::string&) const;

    //! Get variable reference by name hash
    GameVariable* get(Hash) const;

    //! Operator[] overload for getting variable reference by name hash
    GameVariable* operator [] (Hash h) const
    {
        return get(h);
    }

    /*! \brief Get number by hash

        \return true on success, false on failure
    */
    bool getNumber(Hash, GameVariable::NumberType & out) const;
    
    /*! \brief Get integer by hash

        \return true on success, false on failure
    */
    bool getInteger(Hash, GameVariable::IntegerType & out) const;

    /*! \brief Get string by hash

        \return true on success, false on failure
    */
    bool getString(Hash, std::string & out) const;

    //! Get number by hash, or default value on failure
    GameVariable::NumberType getNumberDefault(Hash, GameVariable::NumberType defaultVal) const;
    //! Get integer by hash, or default value on failure
    GameVariable::IntegerType getIntegerDefault(Hash, GameVariable::IntegerType defaultVal) const;
    //! Get string by hash, or default value on failure
    std::string getStringDefault(Hash, const std::string& defaultVal) const;


    //! Get reference to internal ConfigCollection
    ConfigCollection* getConfigCollection()
    {
        return config.get();
    }

    //! Find possible prefixed name matches among all variables
    void getPrefixedMatch(const char* name, std::vector<const char*>& out);

    //! Get total variable count
    size_t getCount();

    //! Load variable values from config group
    void loadFromConfig(const std::vector<GameVariable*>& variables, const char* group);

    //! Load all variables from config groups
    void loadAllFromConfigGroups(char separator);

    //! Constructor
    GameVariableManager();

    //! Destructor
    ~GameVariableManager();
};
