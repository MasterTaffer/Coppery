
#include "game/game.hpp"
#include "script.hpp"
#include <angelscript.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptbuilder/scriptbuilder.h>
#include <scriptarray/scriptarray.h>
#include <scripthandle/scripthandle.h>
#include <scriptmath/scriptmath.h>
#include <scriptdictionary/scriptdictionary.h>
#include <scripthelper/scripthelper.h>

#include "fileOperations.hpp"
#include "log.hpp"
#include <sstream>
#include <cassert>
#include <functional>

#include "control/control.hpp"

#include <GHMAS/entity.h>
#include <GHMAS/stringutils.h>

#include <GHMAS/thread.h>
#include <GHMAS/coroutine.h>
#include <GHMAS/contextpool.h>
#include <GHMAS/random.h>

#include <GHMAS/binarystreambuilder.h>

#include "angelunit.hpp"

#include "regHelper.hpp"

#include "variable.hpp"

//JIT
//#include "as_jit.h"


class FileASReadBinaryStream : public asIBinaryStream
{
    std::unique_ptr<FileHandle> handle;
public:
    FileASReadBinaryStream(std::unique_ptr<FileHandle>& f)
        : handle(std::move(f))
    {

    }

    void Write(const void*, asUINT)
    {

    }

    void Read(void* to, asUINT size)
    {
        if (handle)
            handle->read(to, size);
    }
};


class FileASWriteBinaryStream : public asIBinaryStream
{
    std::unique_ptr<GBVFS::FileWriter> handle;
public:
    FileASWriteBinaryStream(std::unique_ptr<GBVFS::FileWriter>& f)
        : handle(std::move(f))
    {

    }

    void Write(const void* from, asUINT size)
    {
        if (handle)
            handle->write(from, size);
    }

    void Read(void* to, asUINT size)
    {
    }
};

bool ScriptEngine::executeString(const char* s)
{
    if (baseModule != nullptr)
        ExecuteString(ase, s, baseModule, mainContext);
    return true;
}

void GetScriptsInDirectoryRecursive(std::string dirstr, std::vector<std::string> & ret)
{
    GetFilesInDirectoryRecursive(dirstr,".as",ret);
}


void ScriptEngine::messageCallback(const asSMessageInfo *msg)
{
    if (msg->section[0]  != 0)
        Log << msg->section << ":" << msg->row << ":" << msg->col << ": ";

    if (msg->type == asMSGTYPE_WARNING)
    {
        Log << "Warning - " << msg->message << Trace(CHash("AngelScriptWarning"));
    }
    else if (msg->type == asMSGTYPE_ERROR)
    {
        Log << "Error - " << msg->message << Trace(CHash("AngelScriptError"));
    }
    else
    {
        Log << msg->message << Trace(CHash("AngelScript"));
    }
}


void ScriptEngine::exceptionCallback(asIScriptContext *ctx)
{
    int c;
    const char * sn;
    
    auto lnum =ctx->GetExceptionLineNumber(&c, &sn);

    if (sn != nullptr)
        Log << sn;
    Log << ":" << lnum << ":" << c << ": ";

    if (ctx != mainContext)
    {
        Log << "Exception (Sub Context) - ";

    }   
    else
        Log << "Exception - ";
    
    Log << ctx->GetExceptionString();
    Log << Trace(CHash("AngelScriptException"));

    for (size_t i = 0; i < ctx->GetCallstackSize(); i++)
    {
        auto ln = ctx->GetLineNumber(i);
        auto p = ctx->GetFunction(i);
        if (p)
            Log << " In '" << p->GetDeclaration() << "' line " << ln << Trace(CHash("AngelScriptException"));
        else
            Log << " In 'null' line " << ln << Trace(CHash("AngelScriptException"));
    }


    if (ctx != mainContext)
    {
        if (ctx->GetCallstackSize() > 0)
        {
            auto ln = mainContext->GetLineNumber(0);
            auto p = mainContext->GetFunction(0);
            Log << "    Main context in ";
            if (p)
                Log << p->GetScriptSectionName() << " ";
            Log << "line " << ln << Trace(CHash("AngelScriptException"));
        }
    }
}

bool ScriptEngine::validateRST(RequiredScriptType& rst)
{
    bool errors = false;
    auto baseActor = rst.typeInfo;
    for (auto& fund : rst.functions)
    {
        asIScriptFunction *func = baseActor->GetMethodByDecl(fund.declaration.c_str());
        if (fund.ptr)
        {
            Log << "RequiredScriptType function pointer not null on first initialization" << Trace(CHash("Warning"));
            fund.ptr->Release();
            fund.ptr = nullptr;
        }

        if (func == 0)
        {
            Log << "No " << fund.declaration << " found in RST " << rst.name << Trace(CHash("AngelScriptError"));
            errors = true;
        }
        else
        {
            //Only store references if there was no errors
            if (!errors)
            {
                func->AddRef();
                fund.ptr = func;
            }
        }
    }

    for (auto& pd : rst.properties)
    {
        bool found = false;
        for (unsigned int i = 0; i < baseActor->GetPropertyCount(); i++)
        {
            const char* cc;
            int tid;
            int offset = 0;

            baseActor->GetProperty(i, &cc, &tid, nullptr, nullptr, &offset);

            if (tid != pd.type)
                continue;

            if (pd.name != cc)
                continue;

            found = true;

            if (!errors)
            {
                pd.offset = offset;
                pd.index = i;
            }
        }
        if (found == false)
        {
            std::string tname = GetTypeNameByTypeId(ase, pd.type);

            Log << "No property '" << tname << " " << pd.name << "' found in RST " << rst.name << Trace(CHash("AngelScriptError"));
            errors = true;
        }
    }

    if (!errors)
    {
        rst.found = true;
        return true;
    }

    return false;
}

void LineCallback(asIScriptContext *ctx, void*)
{

  const char *scriptSection;
  int col;
  int line = ctx->GetLineNumber(0, &col, &scriptSection);

  Log << "in " << (scriptSection ? scriptSection : "unknown") << ":" << line << ":" << col << Trace(CHash("AngelScriptLineCallback"));
}

void ScriptEngine::initCompilerOnly(Control* c)
{
    compilerOnly = true;
    init(nullptr, c);
}



bool ScriptEngine::initEngine()
{
    asPrepareMultithread();

    ase = asCreateScriptEngine();
    ase->SetUserData((void*)this);
    ase->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, 0);


    coroutineStack = new ASCoroutineStack();

    contextPool = new ASContextPool();
    contextPool->connect(ase);


    //JIT
    //asCJITCompiler* jit = new asCJITCompiler(JIT_NO_SUSPEND);
    //ase->SetEngineProperty(asEP_INCLUDE_JIT_INSTRUCTIONS, 1);
    //ase->SetJITCompiler(jit);

    int r = ase->SetMessageCallback(asMETHOD(ScriptEngine, messageCallback), this, asCALL_THISCALL);
    assert (r >= 0);


    //r = ase->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE,1);
    //assert (r >= 0);

    RegisterScriptArray(ase, true);

    RegisterStdString(ase);

    RegisterScriptHandle(ase);

    RegisterScriptMath(ase);

    RegisterScriptDictionary(ase);

    RegisterScriptAny(ase);

    RegisterThread(ase);
    
    RegisterRandom(ase);


    AngelUnit::RegisterAssertions(ase);
    
    
    //Register "size_t"
    
    if (sizeof(size_t) == 4)
    {
        r = ase->RegisterTypedef("size_t", "uint32");
        assert (r >= 0);
    }
    else
    if (sizeof(size_t) == 8)
    {
        r = ase->RegisterTypedef("size_t", "uint64");
        assert (r >= 0);
    }
    else
    {
        assert("Unsupported sizeof(size_t)" && false);
    }

    //Register Hash functions and type "hash_t"

    if (Hash::Bytes == 4)
    {
        r = ase->RegisterTypedef("hash_t", "uint32");
        assert (r >= 0);
    }
    else
    if (Hash::Bytes == 8)
    {
        r = ase->RegisterTypedef("hash_t", "uint64");
        assert (r >= 0);
    }
    else
    {
        assert("Unsupported Hash size" && false);
    }    
    
    

    r = registerGlobalFunctionAux(this,"hash_t Hash(string)", asFUNCTION(Hash::dyn_hash_string), asCALL_CDECL);
    assert (r >= 0);


    defineVector2();


    defineGameFunctions();

    definePath();

    defineSound();

    defineFormatString();

    defineMap();
    
    defineCollisionFunctions();

    defineFile();

    defineDataTable();

    defineGameVar();
    
    defineDrawables();


    RegisterCoroutine(ase, coroutineStack);

    entitySystemManager = new ASECS::EntitySystemManager();

    entitySystemManager->setLogCallback([](void* uptr, const char* str, int level)
    {
        ScriptEngine* se = (ScriptEngine*) uptr;
        (void)se;
        if (level == ASECS::EntitySystemManager::Info)
            Log << "ESM " << str << Trace(CHash("AngelScript"));
        else
            Log << "ESM " << str << Trace(CHash("AngelScriptWarning"));
    }, (void*)this);

    entitySystemManager->registerEngine(ase);

    //c->callAngelScriptInitHandlers(ase);


    if (!compilerOnly)
    {
        contextPool->setExceptionCallback(asMETHOD(ScriptEngine, exceptionCallback), this, asCALL_THISCALL);
        mainContext = ase->RequestContext();

        //Line callback thing debuggering
        //mainContext->SetLineCallback(asFUNCTION(LineCallback), 0, asCALL_CDECL);

        assert (r >= 0);
    }
    initialized = true;
    (void)(r);
    

    return true;
}

void ScriptEngine::writeEngineConfigToFile(const char* file)
{
    if (!initialized)
    {
        Log << "writeEngineConfigToFile called when script engine not initialized" << Trace(CHash("Warning"));
        
    }
    else
    {
        Log << "Writing AS engine configuration to file " << file << Trace(CHash("General"));
        WriteConfigToFile(ase, file);
    }
}

bool ScriptEngine::initModule(CScriptBuilder* builder, asIScriptModule* mod)
{
    int count = mod->GetObjectTypeCount();

    std::unordered_map<std::string, size_t> rstNames;

    for (size_t i = 0; i < requiredScriptTypes.size(); i++)
    {
        auto& rst = requiredScriptTypes[i];
        rst.typeId = 0;
        rst.found = false;
        rst.typeInfo = nullptr;
        rstNames[rst.name] = i;
    }

    for (int o = 0; o < count; o++)
    {
        asITypeInfo* ot = mod->GetObjectTypeByIndex(o);
        std::string name = ot->GetName();

        auto it = rstNames.find(name);
        if (it != rstNames.end())
        {
            auto& rst = requiredScriptTypes[it->second];

            if (rst.typeInfo)
            {
                Log << "Ignored RST " << name << ": TypeInfo already not null " << Trace(CHash("AngelScriptError"));
                continue;
            }
            else
            {
                Log << "RST " << name << " found"<< Trace(CHash("AngelScript"));
                rst.typeInfo = ot;
                ot->AddRef();
                validateRST(rst);
            }
        }
    }

    for (auto& rst : requiredScriptTypes)
    {
        if (rst.found == false)
        {
            Log << "RST " << rst.name << " not defined" << Trace(CHash("AngelScriptError"));
        }
    }

    entitySystemManager->initEntityClasses(builder);

    if (!compilerOnly)
       mod->ResetGlobalVars();


    count = mod->GetFunctionCount();
    for (int f = 0; f < count; f++)
    {
        asIScriptFunction *func = mod->GetFunctionByIndex(f);
        std::string md = builder->GetMetadataStringForFunc(func);

        if (md == "OnPostNewMap")
        {
            if (func->GetParamCount() != 0)
                continue;
            Log << "[OnPostNewMap] " << func->GetName() << Trace(CHash("AngelScript"));
            func->AddRef();
            postNewMapFunctions.push_back(func);

        }


        if (md == "OnInit")
        {
            if (func->GetParamCount() != 0)
                continue;

            initFunctions.push_back(func);

            func->AddRef();

            Log << "[OnInit] " << func->GetName() << Trace(CHash("AngelScript"));

        }

        if (md == "Test")
        {
            if (func->GetParamCount() != 0)
                continue;

            tests.push_back(func);

            func->AddRef();

            Log << "[Test] " << func->GetName() << Trace(CHash("AngelScript"));

        }
    }
    modulesBuilt += 1;
    if (!baseModule)
        baseModule = mod;

    return true;
}

bool ScriptEngine::loadModuleByteCode(const std::string& source, const std::string& name)
{
    Log << "Loading module " << name << Trace(CHash("AngelScript"));
    if (initialized == false)
    {
        Log << "Failed to initialize module " << name << ": Script engine not initialized" << Trace(CHash("AngelScriptError"));
        return false;
    }

    BinaryStreamBuilder builder;

    auto fs = GetFileStream(source);
    if (!fs)
    {
        Log << "Failed to load byte code from file " << GetFileRealName(source) << Trace(CHash("AngelScriptError"));
        return false;
    }
    FileASReadBinaryStream bs(fs);
    if (builder.LoadModule(ase, name.c_str(), &bs) < 0)
        return false;
    return initModule(&builder, builder.GetModule());
}

bool ScriptEngine::buildModule(const std::vector<std::string>& sourceFiles, const std::string& name)
{
    return buildModuleSave(sourceFiles, name, nullptr);
}

bool ScriptEngine::buildModuleSave(const std::vector<std::string>& sourceFiles, const std::string& name, const char* saveFileName)
{
    Log << "Building module " << name << Trace(CHash("AngelScript"));
    if (initialized == false)
    {
        Log << "Failed to initialize module " << name << ": Script engine not initialized" << Trace(CHash("AngelScriptError"));
        return false;
    }

    if (sourceFiles.size() == 0)
    {
        Log << "Failed to initialize module " << name << ": No input files" << Trace(CHash("AngelScriptError"));
        return false;   
    }

    BinaryStreamBuilder builder;
    int r = builder.StartNewModule(ase, name.c_str());
    if( r < 0 )
    {
        Log << "Failed to initialize module " << name << ": Script builder errorcode "<< r <<  Trace(CHash("AngelScriptError"));
        return false;
    }

    for (auto& s : sourceFiles)
    {
        size_t len;
        std::string realName = GetFileRealName(s);
        char* fcode = GetFileContentsCopy(s, &len);
        if (fcode)
        {
            r = builder.AddSectionFromMemory(realName.c_str(), fcode, len, 0);
            delete[] fcode;
        }
        if (!fcode || r < 0)
        {
            Log << "Failed to load file " << realName << " for module " << name << Trace(CHash("AngelScriptError"));
        }
    }

    r = builder.BuildModule();
    if( r < 0 )
    {
        Log << "Failed to build module " << name  << Trace(CHash("AngelScriptError"));
        return false;
    }

    if (saveFileName)
    {

        auto fs = GetFileWriter(saveFileName);
        if (!fs)
        {   
            Log << "Failed to save byte code to file " << GetFileRealName(saveFileName) << Trace(CHash("AngelScriptError"));
        }
        else
        {
            FileASWriteBinaryStream bs(fs);
            builder.SaveModule(&bs);
        }
    }

    auto* mod = ase->GetModule(name.c_str());
    return initModule(&builder, mod);
}



void ScriptEngine::addRequiredScriptType(const RequiredScriptType& rst)
{
    if (baseModule)
    {
        Log << "addRequiredScriptType called after building ScriptEngine baseModule" << Trace(CHash("Warning"));
    }
    requiredScriptTypes.push_back(rst);
}


const RequiredScriptType* ScriptEngine::getRequiredScriptType(const char* name)
{
    for (auto& rst : requiredScriptTypes)
    {
        if (rst.name == name)
            return &rst;
    }
    return nullptr;
}


void ScriptEngine::initBaseModule()
{
    if (initialized)
    {
        std::string baseModuleName = "base";
        bool recompile = true;
        std::string binaryName = ScriptsCompiledFolder+"/"+baseModuleName;

        if (!compilerOnly)
        {
            auto* varman = engine->getVariableManager();
            int cvar = varman->getIntegerDefault(CHash("Script.Compile"), 2);
            if (cvar == 1)
            {
                if (FileExists(binaryName))
                {
                    recompile = false;
                }
            }
            else if (cvar == 2)
                recompile = true;
            else if (cvar == 0)
                recompile = false;
        }

        if (!recompile)
        {
            loadModuleByteCode(binaryName, baseModuleName);
        }
        else
        {
            std::vector<std::string> files;
            GetScriptsInDirectoryRecursive(ScriptsFolder, files);
            buildModuleSave(files, baseModuleName, binaryName.c_str());
        }
    }
}


void ScriptEngine::init(Engine* e, Control* c)
{
    
    initialized = false;
    baseModule = nullptr;
    ase = nullptr;

    if (compilerOnly)
        Log << "Init in Compiler Only mode" << Trace(CHash("AngelScript"));
    else
        Log << "Init" << Trace(CHash("AngelScript"));

    engine = e;

    initEngine();
}

void ScriptEngine::deInit()
{
    initialized = false;

    for (auto& rst : requiredScriptTypes)
    {
        for (auto& f : rst.functions)
        {
            if (f.ptr)
                f.ptr->Release();
            f.ptr = nullptr;
        }
        if (rst.typeInfo)
        {
            rst.typeInfo->Release();
            rst.typeInfo = nullptr;
        }
    }
    requiredScriptTypes.clear();



    if (mapSpawnObjectCallback)
        mapSpawnObjectCallback->Release();
    mapSpawnObjectCallback = nullptr;

    if (mapParamCallback)
        mapParamCallback->Release();
    mapParamCallback = nullptr;

    for (asIScriptFunction *func : tests)
        func->Release();
    tests.clear();

    for (asIScriptFunction *func : initFunctions)
        func->Release();
    initFunctions.clear();

    for (asIScriptFunction *func : postNewMapFunctions)
        func->Release();
    postNewMapFunctions.clear();

    for (auto* a : tileCollisionCallbacks)
        delete a;  
    tileCollisionCallbacks.clear();

    for (auto& b : scriptCallbackEndStep)
        b.release();
    scriptCallbackEndStep.clear();

    if (coroutineStack)
    {
        coroutineStack->releaseResources();
        delete coroutineStack;
        coroutineStack = nullptr;
    }

    if (mainContext)
    {
        ase->ReturnContext(mainContext);
        mainContext = nullptr;
    }

    if (entitySystemManager)
    {
        entitySystemManager->release();
        delete entitySystemManager;
        entitySystemManager = nullptr;
    }


    if (contextPool)
    {
        contextPool->disconnect();
        delete contextPool;
        contextPool = nullptr;
    }
    asUnprepareMultithread();

    if (ase)
        ase->ShutDownAndRelease();
    ase = nullptr;

    baseModule = nullptr;
    compilerOnly = false;
    engine = nullptr;


}

void ScriptEngine::callInitFunctions()
{
    for (auto* func : initFunctions)
    {
        mainContext->Prepare(func);
        coroutineStack->runMainThread(mainContext);
    }
}


void ScriptEngine::newGame()
{
}

void ScriptEngine::postNewMap()
{
    ase->GarbageCollect();
    for (asIScriptFunction *func : postNewMapFunctions)
    {
        mainContext->Prepare(func);
        coroutineStack->runMainThread(mainContext);
    }
}


void ScriptEngine::update()
{

}

void ScriptEngine::endStep()
{
    scriptCallbackEndStep.remove_if([this](ScriptCallback& cb)
    {
        mainContext->Prepare(cb.callback);

        if (cb.object)
            mainContext->SetObject(cb.object);

        coroutineStack->runMainThread(mainContext);

        bool b = mainContext->GetReturnByte();
        if (b)
            cb.release();
        return b;
    });
}

void ScriptEngine::runCoroutineStack(asIScriptContext* ctx)
{
    coroutineStack->runMainThread(ctx);
}

CircularCallLockHolder::CircularCallLockHolder(bool& lock, const char* msg) : lock(lock)
{
    if (lock)
    {
        denied = true;
        auto* ctx = asGetActiveContext();
        if (!ctx)
        {
            throw std::runtime_error("CircularCallLockHolder detected circular call without AS context");
        }
        else
        {
            if (!msg)
            {
                ctx->SetException("Circular call to protected function detected");
            }
            else
            {
                ctx->SetException(msg);
            }
        }
    }
    else
    {
        lock = true;
        denied = false;
    }
}

bool CircularCallLockHolder::lockDenied()
{
    return denied;
}

CircularCallLockHolder::~CircularCallLockHolder()
{
    if (!denied)
        lock = false;
}
