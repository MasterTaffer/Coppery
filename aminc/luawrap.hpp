#pragma once


#include <tuple>
#include <typeinfo>
#include <string>

#include <lua.hpp>


/*! \file luawrap.hpp
 * 
 * \brief Lua c++ interfacing tools
 * 
 */



/*
    Magical template hax

    Use the LuaCWrap to get a function which can be passed to lua_pushcfunction

    The LuaClosureWrap generates a closure wrapper with a number of upvalues.
    These upvalues are passed as the first arguments to the function.

*/

/*! \def LuaCWrap(wrap)
 * 
 * \brief Wrap a function into a lua callable
 */

#define LuaCWrap(wrap) LuaFunctionWrapper::CFunctionCaller<decltype(&wrap),&wrap>


/*! \def LuaClosureWrap(wrap, count)
 * 
 * \brief Wrap a function into a lua callable with upvalues
 * 
 * The upvalues are used to determine the first \a count arguments
 * for the function to call.
 */

#define LuaClosureWrap(wrap, count) LuaFunctionWrapper::CClosureCaller<decltype(&wrap), (&wrap), (count)>

/*! \def LuaClosureWrapPassLua(wrap, count)
 * 
 * \brief Wrap a function into a lua callable with upvalues and lua pointer
 * 
 * Passes a lua_State pointer in addition to the upvalues. The
 * lua_State pointer should be the first argument.
 */

#define LuaClosureWrapPassLua(wrap, count) LuaFunctionWrapper::CClosureCallerPassLua<decltype(&wrap),&wrap,(count)>

/*
    Advanced stuff:

    struct ArgumentGetter<T>
        Tasked with getting the argument of type T from Lua stack

    struct UpvalueGetter<T>
        Same as above, but with upvalues instead of arguments

    ReturnHandler<T>
        A function that should push a the return value of type T to the
        Lua stack.
*/


/*

    One can observe the fact, that the C++ template system wasn't designed for metaprogramming.


*/

namespace LuaFunctionWrapper
{

    /*
        KNOW THE POWER OF TEMPLATE METAPROGRAMMING!

        (thanks stackoverflow)
        http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer/7858971#7858971

        Generates a goddamn increasing numeric sequence:
        SequenceGenerator<5>::type => NumericSequence<0,1,2,3,4>

        BEHOLD!
    */

    //A rather meta struct: the important information is in the
    //argument pack size_t ...
    template<size_t ...>
    struct NumericSequence
    {

    };

    //This just works:
    //
    //SequenceGenerator<5> :
    //SequenceGenerator<4,4> :
    //SequenceGenerator<3,3,4> :
    //SequenceGenerator<2,2,3,4> :
    //SequenceGenerator<1,1,2,3,4> :
    //SequenceGenerator<0,0,1,2,3,4>

    //See the terminating condition

    template<size_t Target, size_t Num, size_t ... Rest>
    struct SequenceGenerator : SequenceGenerator<Target, Num - 1, Num - 1, Rest ...>
    {

    };

    //End condition: when the first number is Target
    template<size_t Target, size_t ... Rest>
    struct SequenceGenerator<Target, Target, Rest ...>
    {
        typedef NumericSequence<Rest ...> type;
    };

    //Because variables cannot have type "void"
    struct VoidReturn
    {
    };


    //A simple meta struct for some meta function information
    template <typename Return, typename ... Arguments>
    struct FunctionInfo
    {
        typedef typename std::conditional<std::is_same<Return,void>::value,VoidReturn,Return>::type returnValue;
        typedef std::tuple<typename std::remove_const<typename std::remove_reference<Arguments>::type>::type...> arguments;
        static const size_t argumentCount = sizeof...(Arguments);
    };

    //The following function is used just for automatic template argument matching
    template <typename Return, typename ... Args>
    auto FunMainDataHelper(Return(*fn)(Args...)) -> FunctionInfo<Return, Args...>
    {
        typedef FunctionInfo<Return, Args...> FMD;
        return FMD();
    }
    


    //Call function fun with arguments from tuple of type P p
    template <typename Ret, typename P, size_t ... S, typename ... Args>
    Ret CallFunc(NumericSequence<S...>, Ret(*fun)(Args...), const P& p)
    {
        //Some crazy unpackking
        //with 3 arguments unpacks to (std::get<0>(p), std::get<1>(p), std::get<2>(p))

        //UNLIMITED POWER

        return fun(std::get<S>(p) ... );
    }



    //Fun utilities

    //Generate a new tuple from A and a bunch of indices
    template <size_t ... S, typename A>
    auto GetTupleFromIndices(NumericSequence<S...>, A a) -> std::tuple<typename std::tuple_element<S, A>::type...>
    {
        return std::make_tuple(std::get<S>(a) ...);
    }



    //Discards first Count types from the argument pack
    //and defines a tuple type with the remaining ones

    template <size_t Count, typename ... Pack>
    struct PackFrontDiscarder
    {
        typedef std::tuple<Pack...> fullTuple;
        typedef typename SequenceGenerator<Count,std::tuple_size<fullTuple>::value>::type indices;
        typedef decltype(GetTupleFromIndices(indices(),fullTuple())) tail;
    };

    //Takes the first Count types from the argument packs
    //and defines a tuple type with them
    template <size_t Count, typename ... Pack>
    struct PackFrontTaker
    {
        typedef std::tuple<Pack...> fullTuple;
        typedef typename SequenceGenerator<0,Count>::type indices;
        typedef decltype(GetTupleFromIndices(indices(),fullTuple())) head;
    };


    //Similar to above, except these work on tuples

    template <size_t Count, typename Tuple>
    struct TupleFrontDiscarder
    {
        typedef Tuple fullTuple;
        typedef typename SequenceGenerator<Count,std::tuple_size<fullTuple>::value>::type indices;
        typedef decltype(GetTupleFromIndices(indices(),fullTuple())) tail;
    };

    template <size_t Count, typename Tuple>
    struct TupleFrontTaker
    {
        typedef Tuple fullTuple;
        typedef typename SequenceGenerator<0,Count>::type indices;
        typedef decltype(GetTupleFromIndices(indices(),fullTuple())) head;
    };




    //More template goodness

    //Processes Lua argument at index i to type T
    template<typename T>
    struct ArgumentGetter
    {
        static T Get(lua_State* lua, int i)
        {
            static_assert(typeid(T)==typeid(void), "Unhandled Lua argument type.");
            return T();
        }
    };

    template<>
    struct ArgumentGetter<void>
    {
        static void Get(lua_State* lua, int i)
        {
        }
    };

    //Argument getters for integers

    #define MakeIntegerArgumentHandler(TYPE) \
    template<> \
    struct ArgumentGetter<TYPE>\
    {\
        static TYPE Get(lua_State* lua, int i) \
        {\
            return luaL_checkinteger(lua, i);\
        }\
    };\

    MakeIntegerArgumentHandler(char);
    MakeIntegerArgumentHandler(unsigned char);
    MakeIntegerArgumentHandler(int);
    MakeIntegerArgumentHandler(unsigned int);
    MakeIntegerArgumentHandler(short);
    MakeIntegerArgumentHandler(unsigned short);
    MakeIntegerArgumentHandler(long);
    MakeIntegerArgumentHandler(unsigned long);
    MakeIntegerArgumentHandler(long long);
    MakeIntegerArgumentHandler(unsigned long long);

    #undef MakeIntegerArgumentHandler

    //And for them floating points
    #define MakeFloatArgumentHandler(TYPE) \
    template<> \
    struct ArgumentGetter<TYPE>\
    {\
        static TYPE Get(lua_State* lua, int i) \
        {\
            return luaL_checknumber(lua, i);\
        }\
    };\

    MakeFloatArgumentHandler(double);
    MakeFloatArgumentHandler(float);

    #undef MakeFloatArgumentHandler




    //And one for const char*
    template<>
    struct ArgumentGetter<const char*>
    {
        static const char* Get(lua_State* lua, int i)
        {
            return luaL_checkstring(lua,i);
        }
    };

    template<>
    struct ArgumentGetter<std::string>
    {
        static std::string Get(lua_State* lua, int i)
        {
            return luaL_checkstring(lua,i);
        }
    };


    template<>
    struct ArgumentGetter<bool>
    {
        static bool Get(lua_State* lua, int i)
        {
            return lua_toboolean(lua, i);
        }
    };

    //Fat pointer Lua user data, used pass opaque pointers in and out of Lua
    //Note that if a Nullptr is passed into Lua, it might be used for mischief
    struct FatPointer
    {
        constexpr static const char* metaTableName = "LuaWrapper.pointerType";
        size_t type;
        void* ptr;
        static void createMetaTable(lua_State* lua)
        {
            luaL_newmetatable(lua, metaTableName);

            lua_pushliteral(lua, "__index");
            lua_newtable(lua);
            lua_pushliteral(lua, "isNull");
            lua_pushcfunction(lua, l_isNull);

            lua_settable(lua, -3);
            lua_rawset(lua, -3);
            lua_pop(lua, 1);
        };

        static int l_isNull(lua_State* lua)
        {
            FatPointer* fp = (FatPointer*)luaL_checkudata(lua, 1, FatPointer::metaTableName);
            if (fp->ptr == nullptr)
                lua_pushboolean(lua, 1);
            else
                lua_pushboolean(lua, 0);
            return 1;
        };
    };

    //Argument getter for pointers. Type checked pointers. Doesn't support inheritance
    template<typename T>
    struct ArgumentGetter<T*>
    {
        static T* Get(lua_State* lua, int i)
        {
            FatPointer* fp = (FatPointer*)luaL_checkudata(lua, i, FatPointer::metaTableName);
            if (fp->type != typeid(T).hash_code())
            {
                luaL_argerror(lua, i , "Incorrect type");
                return nullptr;
            }
            return (T*)(fp->ptr);
        }
    };


    //You might want to add more of these

    //TODO add one for booleans


    //Takes an argument of type Arg and pushes it into lua_stack
    //also returns the number of items pushed into lua_stack

    template <typename Arg>
    struct ReturnHandler
    {
        static int get(lua_State* lua, Arg a)
        {
            //This doens't handle void returns
            static_assert(typeid(Arg) == typeid(void), "Unhandled Lua return");
            return 0;
        }
    };

    //Void returns are actually handled here
    template <>
    struct ReturnHandler<VoidReturn>
    {
        static int get(lua_State*, VoidReturn)
        {
            return 0;
        }
    };

    //Return handlers for integers
    #define MakeIntegerReturnHandler(TYPE) \
    template <>\
    struct ReturnHandler<TYPE> \
    {\
        static int get(lua_State* lua, TYPE a)\
        {\
            lua_pushinteger(lua, a);\
            return 1;\
        }\
    };

    MakeIntegerReturnHandler(char);
    MakeIntegerReturnHandler(unsigned char);
    MakeIntegerReturnHandler(int);
    MakeIntegerReturnHandler(unsigned int);
    MakeIntegerReturnHandler(short);
    MakeIntegerReturnHandler(unsigned short);
    MakeIntegerReturnHandler(long);
    MakeIntegerReturnHandler(unsigned long);
    MakeIntegerReturnHandler(long long);
    MakeIntegerReturnHandler(unsigned long long);

    #undef MakeIntegerReturnHandler

    //and for floats
    #define MakeFloatReturnHandler(TYPE) \
    template <>\
    struct ReturnHandler<TYPE> \
    {\
        static int get(lua_State* lua, TYPE a)\
        {\
            lua_pushnumber(lua, a);\
            return 1;\
        }\
    };

    MakeFloatReturnHandler(float);
    MakeFloatReturnHandler(double);

    #undef MakeFloatReturnHandler

    //One for boolean return values
    template <>
    struct ReturnHandler<bool>
    {
        static int get(lua_State* lua, bool a)
        {
            lua_pushboolean(lua, a);
            return 1;
        }
    };

    //And std::string 
    template <>
    struct ReturnHandler<std::string>
    {
        static int get(lua_State* lua, const std::string& c)
        {
            lua_pushstring(lua, c.c_str());
            return 1;
        }
    };

    // & const char*
    template <>
    struct ReturnHandler<const char*>
    {
        static int get(lua_State* lua, const char* c)
        {
            lua_pushstring(lua, c);
            return 1;
        }
    };


    //And for type checked FatPointers
    template <typename T>
    struct ReturnHandler<T*>
    {
        static int get(lua_State* lua, T* a)
        {
            //Turn nullptrs to nil
            if (a)
            {
                FatPointer* fp = (FatPointer*) lua_newuserdata(lua, sizeof (FatPointer));
                luaL_getmetatable(lua, FatPointer::metaTableName);
                lua_setmetatable(lua, -2);
                fp->type = typeid(T).hash_code();
                fp->ptr = (void*) a;
            }
            else
            {
                lua_pushnil(lua);
            }
            return 1;
        }
    };

    //TODO add handlers for vectors, arrays and such


    //Upvalue getter, similar to the ArgumentGetter
    template<typename T>
    struct UpvalueGetter
    {
        static T Get(lua_State* lua, int i)
        {
            static_assert(typeid(T)==typeid(void), "Unhandled Lua upvalue type.");
            return T();
        }
    };

    //For the light (and full) opaqua pointer userdata, which is enough for most closures
    template<typename T>
    struct UpvalueGetter<T*>
    {
        static T* Get(lua_State* lua, int i)
        {
            return (T*) lua_touserdata(lua, lua_upvalueindex(i));
        }
    };

    //Simply used for getting the lua_State*
    template<typename T>
    struct MetaGetter
    {
        static T Get(lua_State* lua, int i)
        {
            static_assert(typeid(T)==typeid(void), "Unhandled Lua meta type (first argument of function isn't lua_State*)");
            return T();
        }
    };


    template <>
    struct MetaGetter<lua_State*>
    {
        static lua_State* Get(lua_State* lua, int i)
        {
            return lua;
        }
    };



    //Some more template fun
    //Tuple: a std::tuple<...>
    //Si: starting index
    //Ei: ending index
    template <typename Tuple, template<typename> class Getter, size_t Si, size_t Ei>
    struct TuplePopulator
    {
        //Calls the appropriate GetArgument function
        //and places it into the tuple
        static void PopulateTuple(lua_State* lua, Tuple& t)
        {
            //Get the type at index Si
            typedef typename std::tuple_element<Si, Tuple>::type cctype;

            //Get the argument from Si + 1 (that's how lua works)
            //and place it into index Si
            std::get<Si>(t) = Getter<cctype>::Get(lua, Si + 1);

            //Template recurse
            TuplePopulator<Tuple, Getter, Si + 1, Ei>::PopulateTuple(lua, t);
        }
    };

    //Specialization for the end condition
    template <typename Tuple, template<typename> class Getter, size_t Si>
    struct TuplePopulator<Tuple, Getter, Si, Si>
    {
        static void PopulateTuple(lua_State* lua, Tuple& t)
        {
            //Sure seems like an end condition
        }
    };


    //Because variables cannot be of type "void"

    //Wrestling with the template system
    template <typename T, typename A, typename B, typename C>
    struct VoidMapper
    {
        void doCall(const A& a, const B& b, const C& c)
        {
            value = CallFunc(a,b,c);
        }
        typedef typename std::remove_reference<T>::type internalType;
        internalType value;
    };
    
    template <typename A, typename B, typename C>
    struct VoidMapper<VoidReturn, A, B, C>
    {
        void doCall(A a, B b, const C& c)
        {
            CallFunc(a,b,c);
        }
        typedef VoidReturn internalType;
        internalType value;
    };




    //The wrapper thing
    //typename Function: the type of the function (pointer)
    //Function fun: the function
    template <typename Function, Function fun>
    int CFunctionCaller(lua_State* lua)
    {
        //Get the function info (arguments, return value etc.)
        typedef decltype(FunMainDataHelper(fun)) FInfo;
        //the std::tuple<> type
        typedef typename FInfo::arguments tupleType;
        //generate a numeric sequence 0..tuple_size
        typedef typename SequenceGenerator<0, std::tuple_size<tupleType>::value>::type sequence;
        //the typedef for TuplePopulator, going through arguments 0..tuple_size
        typedef TuplePopulator<tupleType,ArgumentGetter,0,std::tuple_size<tupleType>::value> tp;

        //A whopping 7 lines of runtime code
        //Allocate arguments on stack
        tupleType args;
        //Get the arguments from GetArgument handlers
        tp::PopulateTuple(lua,args);

        //Generate a mapper for voids
        typedef VoidMapper<typename FInfo::returnValue, sequence, Function, tupleType> voidm;

        voidm vm;

        //Call
        vm.doCall(sequence(), fun, args);

        //Return is stored in vm.value
        auto pm = vm.value;
        //Process the return value
        int t = ReturnHandler<typename voidm::internalType>::get(lua, std::move(pm));
        //Return the count of arguments
        return t;
    }

    //Very similar to the above, except this is for a closure
    //typename ... Upvalues: list of closure values
    template <typename Function, Function fun, size_t UpvalueCount>
    int CClosureCaller(lua_State* lua)
    {
        typedef decltype(FunMainDataHelper(fun)) FInfo;

        //The first UpvalueCount arguments to the function are the upvalues

        typedef typename TupleFrontTaker<UpvalueCount, typename FInfo::arguments>::head uvType;

        //TuplePopulator for upvalues uses UpvalueGetter
        typedef TuplePopulator<uvType,UpvalueGetter,0,std::tuple_size<uvType>::value> uvTp;

        //the rest are the arguments

        typedef typename TupleFrontDiscarder<UpvalueCount, typename FInfo::arguments>::tail argType;
        typedef TuplePopulator<argType,ArgumentGetter,0,std::tuple_size<argType>::value> argTp;

        typedef typename SequenceGenerator<0,FInfo::argumentCount>::type sequence;


        argType args;
        argTp::PopulateTuple(lua,args);

        uvType uvs;
        uvTp::PopulateTuple(lua,uvs);

        typedef VoidMapper<typename FInfo::returnValue, sequence, Function, decltype(std::tuple_cat(uvs,args))> voidm;
        voidm vm;
        vm.doCall(sequence(), fun, std::tuple_cat(uvs,args));
        auto pm = vm.value;

        //auto pm = CallFunc(sequence(), fun, std::tuple_cat(uvs,args));
        int t = ReturnHandler<typename voidm::internalType>::get(lua, std::move(pm));
        return t;
    }

    //One more... this passes the lua_State* to the function for a direct access
    //The first argument to the function MUST BE lua_State*, after that comes the
    //upvalues, and then the regular arguments
    template <typename Function, Function fun, size_t UpvalueCount>
    int CClosureCallerPassLua(lua_State* lua)
    {
        typedef decltype(FunMainDataHelper(fun)) FInfo;

        typedef typename TupleFrontTaker<1, typename FInfo::arguments>::head metaType;
        typedef TuplePopulator<metaType,MetaGetter,0,std::tuple_size<metaType>::value> metaTp;

        typedef typename TupleFrontDiscarder<1, typename FInfo::arguments>::tail theRest;

        //The first UpvalueCount arguments to the function are the upvalues

        typedef typename TupleFrontTaker<UpvalueCount, theRest>::head uvType;

        //TuplePopulator for upvalues uses UpvalueGetter
        typedef TuplePopulator<uvType,UpvalueGetter,0,std::tuple_size<uvType>::value> uvTp;

        //the rest are the arguments

        typedef typename TupleFrontDiscarder<UpvalueCount, theRest>::tail argType;
        typedef TuplePopulator<argType,ArgumentGetter,0,std::tuple_size<argType>::value> argTp;

        typedef typename SequenceGenerator<0,FInfo::argumentCount>::type sequence;

        metaType metas;
        metaTp::PopulateTuple(lua,metas);

        argType args;
        argTp::PopulateTuple(lua,args);

        uvType uvs;
        uvTp::PopulateTuple(lua,uvs);


        typedef VoidMapper<typename FInfo::returnValue, sequence, Function, decltype(std::tuple_cat(metas, uvs,args))> voidm;
        voidm vm;
        vm.doCall(sequence(), fun, std::tuple_cat(metas,uvs,args));
        auto pm = vm.value;

        //auto pm = CallFunc(sequence(), fun, std::tuple_cat(metas, uvs,args));
        int t = ReturnHandler<typename voidm::internalType>::get(lua, std::move(pm));
        return t;
    }
    
    //Is used to call Lua functions
    //Arguments are added to Lua stack via ReturnHandler functions
    //luaRef must hold LUA_REGISTRYINDEX for the function
    struct LuaCaller
    {
        lua_State* lua;
        int luaRef;

        template <typename First, typename...Args>
        int addToStack(int i, First f, Args...args)
        {
            i += ReturnHandler<First>::get(lua, f);
            return addToStack(i,args...);
        }

        int addToStack(int i)
        {
            return i;
        }
        template<typename...Args>
        int call(Args...args)
        {
            lua_rawgeti(lua, LUA_REGISTRYINDEX, luaRef);
            int ssize = addToStack(0, args...);
            return lua_pcall(lua,ssize,1,0);
        }

    };
    
    
    
    //Stuff for method metadata
    template <typename Class, typename Return, typename ... Arguments>
    struct MethodInfo
    {
	    typedef Class inClass;
        typedef Return returnValue;
        typedef std::tuple<Arguments...> arguments;
        typedef Return(*funPtr)(Class*, Arguments...);
        static const size_t argumentCount = sizeof...(Arguments);
    };

    template <typename Class, typename Return, typename ... Args>
    auto MethodMainDataHelper(Return(Class::*fn)(Args...)) -> MethodInfo<Class, Return, Args...>
    {
        typedef MethodInfo<Class, Return, Args...> FMD;
        return FMD();
    }

    /*
        Oh, how naïve I was.
	   
        template <typename C, typename T, T fun, typename Ret, typename Args...>
        Ret MethodWrapper(C* p, Args... arguments)
        {
            (p->*fun)(arguments...);
        }
	*/

	

    template<typename C, typename T, T fun, typename Ret, typename ... Args>
    Ret MethodWrapper(C* c, Args... args)
    {
        return (c->*fun)(std::forward<Args>(args)...);
    }

    template<typename T, T t>
    struct MethodWrapperPasser
    {
        typedef T type;
        constexpr static const T value = t;
    };

	template<typename Ftype, Ftype funs, typename OfClass, typename C, typename Ret,  typename ... Args>
	constexpr auto MethodWrapperGenerator(Ret (C::*af)(Args...) ) -> int(*)(lua_State*)
	{
        typedef decltype(&MethodWrapper<OfClass, Ftype, funs, Ret, Args...>) WFType;

        return CFunctionCaller<WFType, &MethodWrapper<OfClass, Ftype, funs, Ret, Args...>>;
	}

    template<typename Ftype, Ftype funs, typename C, typename Ret,  typename ... Args>
    constexpr auto MethodWrapperClosureGenerator(Ret (C::*af)(Args...) ) -> int(*)(lua_State*)
    {
        typedef decltype(&MethodWrapper<C, Ftype, funs, Ret, Args...>) WFType;

        return CClosureCaller<WFType, &MethodWrapper<C, Ftype, funs, Ret, Args...>, 1>;
    }



    template<typename Ftype, Ftype funs, typename OfClass, typename C, typename Ret,  typename ... Args>
    constexpr auto MethodWrapperGenerator(Ret (C::*af)(Args...) const) -> int(*)(lua_State*)
    {
        typedef decltype(&MethodWrapper<OfClass, Ftype, funs, Ret, Args...>) WFType;
        return CFunctionCaller<WFType, &MethodWrapper<OfClass, Ftype, funs, Ret, Args...>>;
    }
}


#define GenericClassMethodWrapper(classfun) LuaFunctionWrapper::MethodWrapperGenerator<decltype(&classfun), &classfun>((&classfun))


#define LuaClassMemberWrap(class, fun) LuaFunctionWrapper::MethodWrapperGenerator<decltype(&class::fun), &class::fun, class>((&class::fun))
#define LuaClassMemberWrapStatic(class, fun) LuaFunctionWrapper::MethodWrapperClosureGenerator<decltype(&class::fun), &class::fun>((&class::fun))

