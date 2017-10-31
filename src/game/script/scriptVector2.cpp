#include "game/engineDefs.hpp"

#include "script.hpp"
#include "vector2.hpp"
#include "color.hpp"

#include <angelscript.h>
#include <cassert>
#include <scriptmath/scriptmathcomplex.h>
#include "log.hpp"


template <typename V2>
struct V2Functions
{
    using DT = typename V2::dimensionType;
    static void constructor(V2* s)
    {
        new (s) V2();
    }

    static void destructor(V2* s)
    {
        s->~V2();
    }

    static void from2(DT a, DT b, V2 * s)
    {

        new(s) V2(a,b);
    }

    static void froml(DT *a, V2 * s)
    {

        new(s) V2(a[0],a[1]);
    }

    static void copy(const V2& from, V2 * s)
    {
        new (s) V2(from);
    }
    
    template< typename T2>
    static T2 castTo(const V2& from)
    {
        return from;
    }
};



//DefVector2
void v2Fromv2(const DefVector2& v2f, DefVector2* s)
{

    new (s) DefVector2(v2f);
}

void v2Constructor(DefVector2* s)
{
    new (s) DefVector2();
}

void v2Destructor(DefVector2* s)
{
    s->~DefVector2();
}


void v2From2(DefVector2Component a, DefVector2Component b, DefVector2 * s)
{

    new(s) DefVector2(a,b);
}

void v2From2l(DefVector2Component *a, DefVector2 * s)
{

    new(s) DefVector2(a[0],a[1]);
}


//Vector2i

void v2iConstructor(Vector2i* s)
{
    new (s) Vector2i();
}

void v2iDestructor(Vector2i* s)
{
    s->~Vector2i();
}

void v2iFrom2(Vector2i::dimensionType a, Vector2i::dimensionType b, Vector2i * s)
{

    new(s) Vector2i(a,b);
}

void v2iFrom2l(Vector2i::dimensionType *a, Vector2i * s)
{

    new(s) Vector2i(a[0],a[1]);
}


//Vector2f, which might be different from Vector2
void v2fFromv2f(const Vector2f& v2f, Vector2f* s)
{
    new (s) Vector2f(v2f);
}

void v2fFromv2d(const DefVector2& v2d, Vector2f* s)
{
    new (s) Vector2f(v2d);
}

void v2fConstructor(Vector2f* s)
{
    new (s) Vector2f();
}

void v2fDestructor(Vector2f* s)
{
    s->~Vector2f();
}

void v2fFrom2f(float a, float b, Vector2f * s)
{

    new(s) Vector2f(a,b);
}

void v2fFrom2fl(float *a, Vector2f * s)
{

    new(s) Vector2f(a[0],a[1]);
}


void cFrom3f(float a, float b, float c, Color * s)
{

    new(s) Color(a,b,c);
}

void cFrom3fl(float *a, Color * s)
{

    new(s) Color(a[0],a[1],a[2]);
}

template<typename UType>
struct v2_funs
{
    void (*constructor)(Vector2<UType>* s);
    void (*destructor)(Vector2<UType>* s);
    void (*from2)(UType a, UType b, Vector2<UType> * s);
    void (*froml)(UType *a, Vector2<UType> * s);
};


template<typename Type>
void define_vector2(asIScriptEngine* ase, const std::string& tname, const std::string& uname, int traits)
{
    using UType = typename Type::dimensionType;
    using funs = V2Functions<Type>;
    int r;

    r = ase->RegisterObjectType(tname.c_str(), sizeof(Type), asOBJ_VALUE | asOBJ_POD | traits | asGetTypeTraits<Type>());
    assert (r >= 0);

    r = ase->RegisterObjectProperty(tname.c_str(),(uname+" x").c_str(), asOFFSET(Type,x));
    assert (r >= 0);

    r = ase->RegisterObjectProperty(tname.c_str(),(uname+" y").c_str(), asOFFSET(Type,y));
    assert (r >= 0);

    r = ase->RegisterObjectBehaviour(tname.c_str(), asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(funs::constructor), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour(tname.c_str(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(funs::destructor), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );
    
    
    r = ase->RegisterObjectBehaviour(tname.c_str(), asBEHAVE_CONSTRUCT, (std::string("void f(const ")+uname+" &in)").c_str(), asFUNCTION(funs::copy), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour(tname.c_str(), asBEHAVE_CONSTRUCT, (std::string("void f(")+uname+","+uname+")").c_str(), asFUNCTION(funs::from2), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour(tname.c_str(), asBEHAVE_LIST_CONSTRUCT, (std::string("void f(const int &in) {")+uname+","+uname+"}").c_str(), asFUNCTION(funs::froml), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod(tname.c_str(), (tname + " opAdd(const "+tname+" &in) const").c_str(), asMETHODPR(Type, operator+, (const Type&) const, Type), asCALL_THISCALL);
    assert (r >= 0);
    r = ase->RegisterObjectMethod(tname.c_str(), (tname + " opSub(const "+tname+" &in) const").c_str(), asMETHODPR(Type, operator-, (const Type&) const, Type), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod(tname.c_str(), ("void opAddAssign(const "+tname+" &in)").c_str(), asMETHODPR(Type, operator+=, (const Type&), void), asCALL_THISCALL);
    assert (r >= 0);
    r = ase->RegisterObjectMethod(tname.c_str(), ("void opSubAssign(const "+tname+" &in)").c_str(), asMETHODPR(Type, operator-=, (const Type&), void), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod(tname.c_str(), (tname + " opMul(const "+tname+" &in) const").c_str(), asMETHODPR(Type, operator*, (const Type&) const, Type), asCALL_THISCALL);
    assert (r >= 0);
    r = ase->RegisterObjectMethod(tname.c_str(), (tname + " opDiv(const "+tname+" &in) const").c_str(), asMETHODPR(Type, operator/, (const Type&) const, Type), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod(tname.c_str(), ("void opMulAssign("+uname+")").c_str(), asMETHODPR(Type, operator*=, (UType), void), asCALL_THISCALL);
    assert (r >= 0);
    r = ase->RegisterObjectMethod(tname.c_str(), ("void opDivAssign("+uname+")").c_str(), asMETHODPR(Type, operator/=, (UType), void), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod(tname.c_str(), (tname + " opMul("+uname+") const").c_str(), asMETHODPR(Type, operator*, (UType) const, Type), asCALL_THISCALL);
    assert (r >= 0);
    r = ase->RegisterObjectMethod(tname.c_str(), (tname + " opDiv("+uname+") const").c_str(), asMETHODPR(Type, operator/, (UType) const, Type), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod(tname.c_str(), "double length() const", asMETHODPR(Type , length, () const, double), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod(tname.c_str(), "double getAngleDegrees() const", asMETHODPR(Type , getAngleDegrees, () const, double), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod(tname.c_str(), (uname+" dot(const "+tname+" &in) const").c_str(), asMETHODPR(Type , dot, (const Type&) const, UType), asCALL_THISCALL);
    assert (r >= 0);

    r = ase->RegisterObjectMethod(tname.c_str(), (tname + " normalize() const").c_str(), asMETHOD(Type , normalize), asCALL_THISCALL);
    assert (r >= 0);
    (void)(r);
}

void ScriptEngine::defineVector2()
{
    int r;


    std::string uname = DefVector2ComponentStringify;

    define_vector2<DefVector2>(ase, "Vector2", uname, asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_ALIGN8);
    define_vector2<Vector2i>(ase, "Vector2i", "int" , asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_ALIGN8);
    
    r = ase->RegisterObjectMethod("Vector2i", "Vector2 opConv()", asFUNCTION(V2Functions<Vector2i>::castTo<DefVector2>), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );
    
    r = ase->RegisterObjectMethod("Vector2", "Vector2i opConv()", asFUNCTION(V2Functions<DefVector2>::castTo<Vector2i>), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );


    /*

        //For defining a separate vector2

        v2_funs<float> funs2;
        funs2.constructor = v2fConstructor;
        funs2.destructor = v2fDestructor;
        funs2.from2 = v2fFrom2f;
        funs2.froml = v2fFrom2fl;
        define_vector2<Vector2f,float>(ase, "GVector2", "float", funs2);

        r = ase->RegisterObjectBehaviour("GVector2", asBEHAVE_CONSTRUCT, "void f(const GVector2 &in)", asFUNCTION(v2fFromv2f), asCALL_CDECL_OBJLAST);
        assert( r >= 0 );

        r = ase->RegisterObjectBehaviour("GVector2", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in)", asFUNCTION(v2fFromv2d), asCALL_CDECL_OBJLAST);
        assert( r >= 0 );
    */


    r = ase->RegisterObjectType("Color",sizeof(Color), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS |  asGetTypeTraits<Color>());
    assert (r >= 0);

    r = ase->RegisterObjectProperty("Color","float r",asOFFSET(Color,r));
    assert (r >= 0);

    r = ase->RegisterObjectProperty("Color","float g",asOFFSET(Color,g));
    assert (r >= 0);

    r = ase->RegisterObjectProperty("Color","float b",asOFFSET(Color,b));
    assert (r >= 0);

    r = ase->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(float,float,float)", asFUNCTION(cFrom3f), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );

    r = ase->RegisterObjectBehaviour("Color", asBEHAVE_LIST_CONSTRUCT, "void f(const int &in) {float,float,float}", asFUNCTION(cFrom3fl), asCALL_CDECL_OBJLAST);
    assert( r >= 0 );

    (void)(r);

}
