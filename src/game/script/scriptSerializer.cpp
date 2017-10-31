//Angelscript object deserialization and xml reading

#include <angelscript.h>
#include <dataser.hpp>
#include <stdint.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptarray/scriptarray.h>
#include "log.hpp"
#include "dataserFile.hpp"
#include "script.hpp"



asIScriptObject* Script_DeSerializeObject(asIScriptEngine* ase, asIScriptContext* ctx, asITypeInfo* type, Dataser::Table* table);

bool Script_DeSerializeObjectProperty(asIScriptEngine* ase, asIScriptContext* ctx, int typedid, void* add, const char* propname, Dataser::Table* table);

//Write a Dataser::Value to the address

template <typename T>
bool CastToInt(Dataser::Value& val, void* ptr)
{
    T* cptr = (T*)ptr;
    T vdr;
    auto m = val.getInteger();
    if (m.isNull)
    {
        auto m2 = val.getDouble();
        if (m2.isNull)
            return false;
        else
            vdr = m2.access();
    }
    else
        vdr = m.access();
    *cptr = vdr;
    return true;
}


template <typename T>
bool CastToFloat(Dataser::Value& val, void* ptr)
{
    T* cptr = (T*)ptr;
    T vdr;
    
    auto m2 = val.getDouble();
    if (m2.isNull)
        return false;
    else
        vdr = m2.access();
    *cptr = vdr;
    return true;
}

//Angelscript helper function: get type name by typeid
std::string GetTypeNameByTypeId(asIScriptEngine* ase, int typedid)
{
    if (typedid < 12)
    {
        if (typedid == asTYPEID_INT8) return "int8";
        else if (typedid == asTYPEID_UINT8) return "uint8";
        else if (typedid == asTYPEID_INT16) return "int16";
        else if (typedid == asTYPEID_UINT16) return "uint16";
        else if (typedid == asTYPEID_INT32) return "int32";
        else if (typedid == asTYPEID_UINT32) return "uint32";
        else if (typedid == asTYPEID_INT64) return "int64";
        else if (typedid == asTYPEID_UINT64) return "uint64";
        else if (typedid == asTYPEID_FLOAT) return "float";
        else if (typedid == asTYPEID_DOUBLE) return "double";
        else if (typedid == asTYPEID_BOOL) return "bool";
    }
    else
    {
        asITypeInfo* proptype = ase->GetTypeInfoById(typedid);
        if (proptype)
        {
            std::string s = proptype->GetNamespace();
            s += "::";
            s += proptype->GetName();
            return s;
        }
    }
    return "void";
}

//Get type size by typeid
size_t GetTypeSizeByTypeId(asIScriptEngine* ase, int typedid)
{
    if (typedid < 12)
    {
        if (typedid == asTYPEID_INT8) return 1;
        else if (typedid == asTYPEID_UINT8) return 1;
        else if (typedid == asTYPEID_INT16) return 2;
        else if (typedid == asTYPEID_UINT16) return 2;
        else if (typedid == asTYPEID_INT32) return 4;
        else if (typedid == asTYPEID_UINT32) return 4;
        else if (typedid == asTYPEID_INT64) return  8;
        else if (typedid == asTYPEID_UINT64) return 8;
        else if (typedid == asTYPEID_FLOAT) return 8;
        else if (typedid == asTYPEID_DOUBLE) return 8;
        else if (typedid == asTYPEID_BOOL) return 4;
    }
    else
    {
        asITypeInfo* proptype = ase->GetTypeInfoById(typedid);
        if (proptype)
            return proptype->GetSize();
    }
    return 0;
}

//Deserialize a CSCriptArray
bool Script_DeSerializeArray(asIScriptEngine* ase, asIScriptContext* ctx, CScriptArray* arr, asITypeInfo* array, int subtype, Dataser::Table* table)
{
    //The CScriptArray stores everything but primitives as references
    bool isReference = false;
    size_t size = GetTypeSizeByTypeId(ase, subtype);
    if (subtype >= 12) //Angelscript typeids above 11 are not primitive types
    {
        isReference = true;
        size = sizeof(intptr_t);
    }

    //NO HANDLE DESERIALIZATION
    if (subtype & asTYPEID_OBJHANDLE)
        return false;

    //If something went wrong already (void array?)
    if (size == 0)
        return false;

    //Clear the array
    arr->Resize(0);

    int index = 0;
    while (true)
    {
        //Use zero index converted to string as the key
        std::string key = std::to_string(index);
        auto v = table->get(key);
        if (v.isNull()) //If the key isn't found
            break; 

        //Otherwise, add an index 
        arr->Resize(index+1);

        if (isReference) //If we have a reference: arr->GetBuffer() + size*index points to a pointer
            Script_DeSerializeObjectProperty(ase,ctx,subtype,*(void**)(((char*)arr->GetBuffer())+size*index),key.c_str(),table);
        else //otherwise, it's a plain value
            Script_DeSerializeObjectProperty(ase,ctx,subtype,(void*)(((char*)arr->GetBuffer())+size*index),key.c_str(),table);
        index += 1;
    }
    return true;
}

bool Script_DeSerializeObjectProperty(asIScriptEngine* ase, asIScriptContext* ctx, int typedid, void* add, const char* propname, Dataser::Table* table)
{
    if (typedid < 12)
    {
        //Primitive type
        auto val = table->get(propname);
        if (val.isNull())
            return false;
        if (typedid == asTYPEID_BOOL)
        {
            auto vm = val.getDouble();
            if (vm.isNull)
                *(bool*)add = false;
            else
                *(bool*)add = (vm.access() != 0);
            return true;
        }
        if (typedid == asTYPEID_INT8) return CastToInt<int8_t>(val, add);
        else if (typedid == asTYPEID_UINT8) return CastToInt<uint8_t>(val, add);
        else if (typedid == asTYPEID_INT16) return CastToInt<int16_t>(val, add);
        else if (typedid == asTYPEID_UINT16) return CastToInt<uint16_t>(val, add);
        else if (typedid == asTYPEID_INT32) return CastToInt<int32_t>(val, add);
        else if (typedid == asTYPEID_UINT32) return CastToInt<uint32_t>(val, add);
        else if (typedid == asTYPEID_INT64) return CastToInt<int64_t>(val, add);
        else if (typedid == asTYPEID_UINT64) return CastToInt<uint64_t>(val, add);
        else if (typedid == asTYPEID_FLOAT) return CastToFloat<float>(val, add);
        else if (typedid == asTYPEID_DOUBLE) return CastToFloat<double>(val, add);
        
        return false;
    }
    else
    {
        //Special case: string
        if (typedid == ase->GetTypeIdByDecl("string"))
        {
            auto val = table->get(propname);
            auto op = val.getString();
            if (!op.isNull)
            {
                auto cptr = (std::string*)add;
                (*cptr) = op.access();
                return true;
            }
            return false;
        }

        auto val = table->getTable(propname);
        if (val == nullptr)
            return false;

        if (typedid & asTYPEID_OBJHANDLE) //Do not deserialize handles
            return false;

        asITypeInfo* proptype = ase->GetTypeInfoById(typedid);

        //Special case: array
        //Not sure if angelscrip allows more convenient way of checking
        //templated types

        if (typedid & asTYPEID_TEMPLATE)
        {
            if (proptype->GetSubTypeCount() == 1)
            {
                //We need the module
            //	auto se = (ScriptEngine*)ase->GetUserData();
                auto p = proptype->GetSubTypeId();

                //The subtype name as a string
                std::string tname = GetTypeNameByTypeId(ase,p);

                //Search for a type "array<subtype>" in the module

                auto arrtype = std::string("array<")+tname+">";
                auto tidd = ase->GetTypeIdByDecl(arrtype.c_str());
                if (tidd <= 0)
                {
                    auto* fun = ctx->GetFunction();
                    if (fun)
                    {
                        auto* mod = fun->GetModule();
                        tidd = mod->GetTypeIdByDecl(arrtype.c_str());

                    }
                }
                

                
                
                //and compare
                if (typedid == tidd)
                {
                    CScriptArray* cptr = (CScriptArray*) add;

                    return Script_DeSerializeArray(ase, ctx, cptr, proptype, p, val);
                }
            }
        }
        
        //Try to deserialize another object

        asIScriptObject* nobj = nullptr;

        nobj = Script_DeSerializeObject(ase, ctx, proptype, val);

        if (nobj == nullptr)
            return false;
        auto cptr = (asIScriptObject*)add;

        cptr->CopyFrom(nobj);
        nobj->Release();
        return true;
    }
}


asIScriptObject* Script_DeSerializeObject(asIScriptEngine* ase, asIScriptContext* ctx, asITypeInfo* type, Dataser::Table* table)
{
    //Type flags
    auto flags = type->GetFlags();

    //Do not deserialize stuff that isn't a script object
    if (!(flags & asOBJ_SCRIPT_OBJECT))
        return nullptr;

    //If the type has no properties
    if (type->GetSize() == 0)
        return nullptr;

    //Construct a new one
    asIScriptFunction* fact = type->GetFactoryByIndex(0);

    //Assert that the factory exists & has no parameters
    if (fact == nullptr)
        return nullptr;

    if (fact->GetParamCount() > 0)
    {
        return nullptr;
    }
    ctx->Prepare(fact);
    ctx->Execute();
    asIScriptObject* obj = *(asIScriptObject**)ctx->GetAddressOfReturnValue();
    
    if (obj == nullptr)
        return nullptr;

    obj->AddRef();
    for (size_t i = 0; i < obj->GetPropertyCount(); i++)
    {
        //Go through the properties

        const char* name = obj->GetPropertyName(i);
        int typedid = obj->GetPropertyTypeId(i);;
        void* offset = obj->GetAddressOfProperty(i);
    
        Script_DeSerializeObjectProperty(ase, ctx, typedid, offset, name, table);
    }
    return obj;
}


//Object that wraps a Dataser::Table for the scripts
class ScriptDataserTable
{
public:
    Dataser::Table* table = nullptr;
    ScriptDataserTable* parent = nullptr;
    std::vector<ScriptDataserTable*> references;
    int ref = 1;
    ScriptDataserTable()
    {
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
            if (parent)
            {
                parent->removeReferenceTo(this);
                parent->release();
            }
            else
                if (table)
                    delete table;

            delete this;
        }
    }


    void removeReferenceTo(ScriptDataserTable* sdt)
    {
        references.erase(std::remove(references.begin(), references.end(), sdt), references.end());
    }

    ScriptDataserTable* getTable(const std::string& i)
    {
        if (table)
        {
            auto nt = table->getTable(i);
            if (nt == nullptr)
                return nullptr;

            for (auto r: references)
            {
                if (r->table == nt)
                {
                    r->addRef();
                    return r;
                }
            }

            auto p = new ScriptDataserTable();
            addRef();
            p->parent = this;
            p->table = nt;
            references.push_back(p);
            return p;
        }
        return nullptr;
    }


    bool getGeneric(const std::string& name, void* i, int tid)
    {
        auto etable = table;
        if (etable)
        {
            std::string np = name;
            size_t pos;
            while ((pos = np.find('/')) != np.npos)
            {
                etable = etable->getTable(np.substr(0, pos));
                if (etable == nullptr)
                    return false;
                np = np.substr(pos + 1);
            }
            auto ctx = asGetActiveContext();
            auto ase = ctx->GetEngine();
            return Script_DeSerializeObjectProperty(ase, ctx, tid, i, np.c_str(), etable);
        }
        return false;
    }

    template <typename T>
    void set(const std::string& name, T value)
    {
        auto etable = table;
        //Log << name << " <- " << value << Message();
        if (etable)
        {
            std::string np = name;
            size_t pos;
            while ((pos = np.find('/')) != np.npos)
            {
                etable = etable->getTable(np.substr(0, pos));
                if (etable == nullptr)
                {
                    auto ctx = asGetActiveContext();
                    ctx->SetException("Invalid path");
                    return;
                }
                np = np.substr(pos + 1);
            }

            auto& r = etable->getOrInsert(np);
            r.set(value);
            return;
        }
        auto ctx = asGetActiveContext();
        ctx->SetException("Set access on null table");
    }

    void setString(const std::string& name, const std::string& value)
    {
        set<const std::string&>(name, value);
    }

    void setDouble(const std::string& name, double value)
    {
        set<double>(name, value);
    }

    void setInt(const std::string& name, int64_t value)
    {
        set<int64_t>(name, value);
    }

    bool getString(const std::string& name, void* i)
    {
        auto ctx = asGetActiveContext();
        auto ase = ctx->GetEngine();
        return getGeneric(name, i, ase->GetTypeIdByDecl("string"));
    }

    bool getDouble(const std::string& name, void* i)
    {
        return getGeneric(name, i, asTYPEID_DOUBLE);
    }

    bool getInt(const std::string& name, void* i)
    {
        return getGeneric(name, i, asTYPEID_INT64);
    }

    void remove(const std::string& key)
    {
        if (!exists(key))
        {
            auto ctx = asGetActiveContext();
            ctx->SetException("Failed to remove null value");
            return;
        }
        table->get(key).remove();
    }


    void newTable(const std::string& key)
    {
        if (!table)
        {
            auto ctx = asGetActiveContext();
            ctx->SetException("Set access on null table");
            return;
        }
        if (exists(key))
        {
            auto ctx = asGetActiveContext();
            ctx->SetException("Failed to insert over already existing value");
            return;
        }

        table->newTable(key);
    }



    bool exists(const std::string& key)
    {
        if (table)
        {
            return !(table->get(key).isNull());
        }
        return false;
    }

    bool isNull()
    {
        return (table == nullptr);
    }

    ScriptDataserTable* clone()
    {
        
        if (table)
        {
            auto ntable = table->clone();
            if (!ntable)
                return nullptr;

            auto* nt = new ScriptDataserTable();
    
            nt->table = ntable;
            return nt;
        }
        return nullptr;
    }

};

//Deserialize a table to the passed object
bool Script_DeSerializeTo(ScriptDataserTable* tbl, void* ref, int tid)
{
    auto ctx = asGetActiveContext();
    auto ase = ctx->GetEngine();

    if (tbl->isNull())
    {
        ctx->SetException("Null table passed to DeSerialize");
        return false;
    }
    if ((tid & asTYPEID_SCRIPTOBJECT))
    {
        asITypeInfo* typinfo = ase->GetTypeInfoById(tid & (~asTYPEID_OBJHANDLE));

        //Push the state, deserialization might construct more script objects
        ctx->PushState();
        
        asIScriptObject* nobj = Script_DeSerializeObject(ase,ctx,typinfo,tbl->table);

        ctx->PopState();

        //If serialization didn't work out
        if (!nobj)
        {
            return false;
        }


        if (tid & asTYPEID_OBJHANDLE)
        {
            auto handleptr = (asIScriptObject**)(ref);

            if (*handleptr) 
            {
                (*handleptr)->Release();
            }

            (*handleptr) = nobj;
        }
        else
        {
            asIScriptObject* objptr;
            objptr = (asIScriptObject*)(ref);
            objptr->CopyFrom(nobj);
            nobj->Release();
        }

        return true;
    }
    else
    {
        ctx->SetException("DeSerialize must be called with a reference to a script class");

        return false;
    }
}

void SaveDataTable(ScriptDataserTable* tbl, const std::string& i)
{
    if (tbl->isNull())
    {
        auto ctx = asGetActiveContext();
        ctx->SetException("Null table passed to SaveDataTable");
        return;
    }
    Dataser::WriteXMLToFile(tbl->table, i);

}

ScriptDataserTable* LoadDataTable(const std::string& i)
{
    ScriptDataserTable* nt = new ScriptDataserTable();
    
    nt->table = Dataser::ReadFromFile(i);

    if (nt->table == nullptr)
    {
        nt->release();
        return nullptr;
    }

    return nt;
}

ScriptDataserTable* CreateEmptyDataTable()
{
    ScriptDataserTable* nt = new ScriptDataserTable();
    nt->table = new Dataser::Table();
    return nt;
}


void ScriptEngine::defineDataTable()
{
    int r = 0;
    r = ase->RegisterObjectType("DataTable",0, asOBJ_REF);
    assert (r >= 0);

    ase->RegisterObjectBehaviour("DataTable", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptDataserTable,addRef), asCALL_THISCALL);
    assert( r >= 0 );

    ase->RegisterObjectBehaviour("DataTable", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptDataserTable,release), asCALL_THISCALL);
    assert( r >= 0 );

    r = ase->RegisterObjectMethod("DataTable", "DataTable@ getTable(const string &in)", asMETHOD(ScriptDataserTable, getTable), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "DataTable@ clone()", asMETHOD(ScriptDataserTable, clone), asCALL_THISCALL);
    assert(r >= 0);


    r = ase->RegisterObjectMethod("DataTable", "bool getInt(const string &in, int64 &out)", asMETHOD(ScriptDataserTable, getInt), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "bool getDouble(const string &in, double &out)", asMETHOD(ScriptDataserTable, getDouble), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "bool getString(const string &in, string &out)", asMETHOD(ScriptDataserTable, getString), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "void setInt(const string &in, int64)", asMETHOD(ScriptDataserTable, setInt), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "void setDouble(const string &in, double)", asMETHOD(ScriptDataserTable, setDouble), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "void setString(const string &in, const string &in)", asMETHOD(ScriptDataserTable, setString), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "void newTable(const string &in)", asMETHOD(ScriptDataserTable, newTable), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "void remove(const string &in)", asMETHOD(ScriptDataserTable, remove), asCALL_THISCALL);
    assert(r >= 0);

    r = ase->RegisterObjectMethod("DataTable", "bool exists(const string &in)", asMETHOD(ScriptDataserTable, exists), asCALL_THISCALL);
    assert(r >= 0);


    r = ase->SetDefaultNamespace("DataTable");
    assert(r >= 0);

    r = ase->RegisterGlobalFunction("void Save(DataTable&, const string &in)", asFUNCTION(SaveDataTable), asCALL_CDECL);
    assert(r >= 0);

    r = ase->RegisterGlobalFunction("DataTable@ Load(const string &in)", asFUNCTION(LoadDataTable), asCALL_CDECL);
    assert (r >= 0);

    r = ase->RegisterGlobalFunction("DataTable@ Create()", asFUNCTION(CreateEmptyDataTable), asCALL_CDECL);
    assert(r >= 0);

    r = ase->RegisterGlobalFunction("bool DeSerialize(DataTable& db, ?&out)", asFUNCTION(Script_DeSerializeTo), asCALL_CDECL);
    assert (r >= 0);

    r = ase->SetDefaultNamespace("");
    assert(r >= 0);
    (void)(r);

}
