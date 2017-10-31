
#include <angelscript.h>
#include <string>
#include <vector>
#include <unordered_map>

#include <GHMAS/coroutine.h>

#include "script.hpp"
#include "log.hpp"
#include "angelunit.hpp"

#include <fstream>

bool ScriptEngine::runTests(const std::string& outputFile)
{
    if (!initialized)
        return false;

    std::unordered_map<std::string, size_t> suiteIndices;
    std::vector<AngelUnit::TestSuite> suites;
    
    auto getSuite = [&](const std::string& name) -> AngelUnit::TestSuite&
    {
        auto it = suiteIndices.find(name);
        if (it == suiteIndices.end())
        {
            AngelUnit::TestSuite ts;
            ts.name = name;
            suiteIndices[name] = suites.size();
            suites.push_back(std::move(ts));

            return *(suites.rbegin());
        }
        return suites[it->second];

    };

    bool allSuccess = true;

    int i = 0;
    int fails = 0;
    Log << "Running tests" << Trace(CHash("AngelScriptTesting"));

    auto* ctx = ase->RequestContext();
    for (auto* func : tests)
    {
        std::string testName;
        testName = func->GetName();

        std::string suiteName;
        auto* classType = func->GetObjectType();
        if (classType)
        {
            suiteName = classType->GetName();
        }
        else
        {
            if (func->GetNamespace())
            {
                suiteName = func->GetNamespace();
            }
            if (suiteName == "")
            {
                suiteName = func->GetModuleName();
            }
        }

        Log << "Running \"" << suiteName << "::" << testName << "\"" << Trace(CHash("AngelScriptTesting"));
        auto& suite = getSuite(suiteName);

        suite.tests.push_back(AngelUnit::TestResults());
        AngelUnit::TestResults& tr = *(suite.tests.rbegin());
        bool success = AngelUnit::RunTest(&tr, func, ctx, nullptr, coroutineStack);
        allSuccess = success & allSuccess;

        if (success)
            Log << "OK ";
        else
        {
            Log << "FAILED ";
            fails += 1;
        }

        Log << "\"" << suiteName << "::" << testName << "\"" << Trace(CHash("AngelScriptTesting"));
        i+= 1;


    }
    ase->ReturnContext(ctx);

    //If there are no tests, don't output
    if (i != 0)
    {
        std::ofstream file = std::ofstream(outputFile.c_str());
        GenerateJUnitXML(suites, file);
        
        Log << "Ran " << i << " tests ";
        if (fails)
        {
            Log << "with " << fails << " failure(s)" << Trace(CHash("AngelScriptTesting"));
        }
        else
            Log << "with no failures" << Trace(CHash("AngelScriptTesting"));
    }
    else
        Log << "No tests ran" << Trace(CHash("AngelScriptTesting"));
    return allSuccess;
}
