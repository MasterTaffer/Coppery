#pragma once
#include <functional>
#include <utility>


#ifndef CROSSFUNCTION_NO_LUA
#include "luawrap.hpp"
#include "log.hpp"
#endif


//! Utilities for CrossFunction
namespace CrossFunctionUtil
{

//! Template structure for retrieving a return value from the Lua stack
template <typename RetType>
struct DoLuaReturnGet
{
    //! Function for retrieving a return value from the Lua stack 
    static RetType returnGet(lua_State* lua)
    {
        RetType temp = LuaFunctionWrapper::ArgumentGetter<RetType>::Get(lua, -1);
        lua_pop(lua, 1);
        return temp;
    }
};

//!\cond

//Specialize for void: C++ doesn't approve of variables with type void
template <>
struct DoLuaReturnGet<void>
{
    static void returnGet(lua_State* lua)
    {
        lua_pop(lua, 1);
        return;
    }
};
//!\endcond

}


/*! \brief A "cross-language" function wrapper
 * 
 * Can contain a C++ function or a Lua function with opaque interface.
 */

template <typename Ret, typename ...Args>
class CrossFunction
{
public:
    
    //! Type of the C++ function stored
    typedef std::function<Ret(Args...)> ClosureType;
private:

    enum Type
    {
        Null,
        LuaFunction,
        Closure
    };
    Type type = Null;


    #ifndef CROSSFUNCTION_NO_LUA
    LuaFunctionWrapper::LuaCaller luaCaller;
    #endif

    ClosureType closure;


    template <typename ... Fargs>
    Ret luaCall(Fargs&&...args)
    {
        #ifndef CROSSFUNCTION_NO_LUA
        if (luaCaller.call(std::forward<Fargs>(args)...))
        {
            Log << lua_tostring(luaCaller.lua,-1) << Trace(CHash("Warning"));
            lua_pop(luaCaller.lua,1);
            return Ret();
        }

        return CrossFunctionUtil::DoLuaReturnGet<Ret>::returnGet(luaCaller.lua);
        #else
        return Ret();
        #endif
    }


public:
    
    //! Default contructor
    CrossFunction()
    {
    }

    //! Constructor from std::function
    CrossFunction(const ClosureType& fun)
    {
        setClosure(fun);
    }

    //! Constructor from plain function pointer
    CrossFunction(const Ret(*fun)(Args...))
    {
        setClosure(ClosureType(fun));
    }


    #ifndef CROSSFUNCTION_NO_LUA
    
    /*! Set the function to a Lua function
     * 
     * \p ref must be a LUA_REGISTRYINDEX reference to the function in the 
     * lua_State \p l
     */
    void setLuaFunction(lua_State* l, int ref)
    {
        luaCaller.lua = l;
        luaCaller.luaRef = ref;

        type = LuaFunction;
    }
    #endif

    //! Set the function from std::function 
    void setClosure(const ClosureType& fun)
    {
        closure = fun;
        type = Closure;
    }



    //! Call the function with arguments
    template <typename ... Fargs>
    Ret call(Fargs&&...args)
    {
        if (type == Null)
            return Ret();
        if (type == Closure)
            return closure(std::forward<Fargs>(args)...);
        if (type == LuaFunction)
            return luaCall(std::forward<Fargs>(args)...);
        return Ret();
    }

    //! Call the function with arguments
    template <typename ... Fargs>
    Ret operator()(Fargs&&...args)
    {
        return call(std::forward<Fargs>(args)...);
    }

    //! Returns true if a function is callable
    operator bool()
    {
        return (type != Null);
    }

};



#ifndef CROSSFUNCTION_NO_LUA

#include "log.hpp"

#include <vector>
#include "vector2.hpp"
#include "matrix.hpp"

namespace LuaFunctionWrapper
{
    //! ArgumentGetter specialization for CrossFunction
    template<typename A, typename ... B>
    struct ArgumentGetter<CrossFunction<A, B...> > 
    {
        //! Get the value from Lua stack
        static ::CrossFunction<A, B...> Get(lua_State* lua, int i)
        {
            if (!lua_isfunction(lua,i))
            {
                luaL_argerror(lua, i , "Incorrect type, expected a function");
            }
            lua_pushvalue(lua,i);
            int r = luaL_ref(lua, LUA_REGISTRYINDEX);
            ::CrossFunction<A, B...> cb;
            cb.setLuaFunction(lua, r);
            return cb;
        }
    };

    //! ArgumentGetter specialization for Vector2
    template<typename T>
    struct ArgumentGetter<Vector2<T>> 
    {
        //! Get the value from Lua stack
        static Vector2<T> Get(lua_State* lua, int i)
        {
            Vector2<T> dv;
            const char* msg = "Incorrect type, expected a Vector2 table";
            if (!lua_istable(lua,i))
            {
                luaL_argerror(lua, i , msg);
                return dv;
            }
            lua_pushinteger(lua,1);
            lua_gettable(lua,i);
            if (!lua_isnumber(lua,-1))
            {
                lua_pop(lua,1);
                luaL_argerror(lua, i , msg);
                return dv;
            }
            dv.x = lua_tonumber(lua,-1);
            lua_pop(lua,1);

            lua_pushinteger(lua,2);
            lua_gettable(lua,i);
            if (!lua_isnumber(lua,-1))
            {
                lua_pop(lua,1);
                luaL_argerror(lua, i , msg);
                return dv;
            }
            dv.y = lua_tonumber(lua,-1);
            lua_pop(lua,1);

            return dv;
        }
    };


    //! ArgumentGetter specialization for Matrix4
    template<>
    struct ArgumentGetter<Matrix4> 
    {
        //! Get the value from Lua stack
        static Matrix4 Get(lua_State* lua, int i)
        {
            Matrix4 mat;
            const char* msg = "Incorrect type, expected an array with 16 number elements";
            if (!lua_istable(lua,i))
            {
                luaL_argerror(lua, i , msg);
            }

            
            for (int index = 1; index <= 16; ++index)
            {
                lua_pushinteger(lua,index);
                lua_gettable(lua,i);

                if (!lua_isnumber(lua,-1))
                {
                    lua_pop(lua,1);
                    luaL_argerror(lua, i , msg);
                    break;
                }
                else
                {
                    mat.data[index] = lua_tonumber(lua, -1);
                    lua_pop(lua,1);
                }

            }
            return mat;
        
        }
    };


    
    //! ArgumentGetter specialization for std::vector
    template<typename T>
    struct ArgumentGetter<std::vector<T>> 
    {
        //! Get the value from Lua stack
        static std::vector<T> Get(lua_State* lua, int i)
        {
            std::vector<T> dv;
            const char* msg = "Incorrect type, expected an array";
            if (!lua_istable(lua,i))
            {
                luaL_argerror(lua, i , msg);
                return dv;
            }

            int index = 1;
            for (;;)
            {
                lua_pushinteger(lua,index);
                lua_gettable(lua,i);
                if (lua_isnil(lua,-1))
                {
                    lua_pop(lua,1);
                    break;
                }
                else
                {
                    T t = ArgumentGetter<T>::Get(lua, lua_gettop(lua));
                    dv.push_back(t);
                    lua_pop(lua,1);
                }
                index += 1;

            }
            return dv;
        
        }
    };

    //! ReturnHandler specialization for Vector2
    template <typename T>
    struct ReturnHandler<Vector2<T>>
    {
        //! Push the value to Lua stack
        static int get(lua_State* lua, Vector2<T> v)
        {
            lua_newtable(lua);
            lua_pushinteger(lua,1);
            lua_pushnumber(lua,v.x);
            lua_settable(lua,-3);
            lua_pushinteger(lua,2);
            lua_pushnumber(lua,v.y);
            lua_settable(lua,-3);
            return 1;
        }
    };

    //! ReturnHandler specialization for Vector2i
    template <>
    struct ReturnHandler<Vector2i>
    {
        //! Push the value to Lua stack
        static int get(lua_State* lua, Vector2i v)
        {
            lua_newtable(lua);
            lua_pushinteger(lua,1);
            lua_pushinteger(lua,v.x);
            lua_settable(lua,-3);
            lua_pushinteger(lua,2);
            lua_pushinteger(lua,v.y);
            lua_settable(lua,-3);
            return 1;
        }
    };


    //! ReturnHandler specialization for std::vector
    template <typename T>
    struct ReturnHandler<const std::vector<T>&>
    {
        //! Push the value to Lua stack
        static int get(lua_State* lua, const std::vector<T>& vec)
        {
            lua_newtable(lua);
            for (unsigned int i = 0; i < vec.size(); i++)
            {
                lua_pushinteger(lua,i+1);
                int vs = ReturnHandler<T>::get(lua, vec[i]);
                if (vs == 0)
                {
                    lua_pop(lua,1);
                    continue;
                }
                if (vs > 2)
                {
                    lua_pop(lua,1);
                    luaL_error(lua, "Internal error: Array element ReturnHandler with multiple returns not supported.");
                }
                lua_settable(lua,-3);
            }
            return 1;
        }
    };
    
    //! ReturnHandler specialization for std::vector
    template <typename T>
    struct ReturnHandler<std::vector<T>>
    {
        //! Push the value to Lua stack
        static int get(lua_State* lua, std::vector<T> vec)
        {
            return ReturnHandler<const std::vector<T>&>::get(lua, vec);
        }
    };
}

#endif
