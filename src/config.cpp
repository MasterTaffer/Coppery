#include <fstream>
#include <sstream>
#include <algorithm>

#include "log.hpp"
#include "config.hpp"
#include "stringUtil.hpp"

bool ConfigGroup::parseLine(const std::string& line)
{
    if (line.size() == 0)
        return false;

    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos)
        return false;

    if (line[0] == '#') //Comment
        return false;

    if (line.find('=',equalPos+1) != std::string::npos)
    {
        //TODO
        //Should probably report this error
        return false;
    }
    std::string l,r;
    l = line.substr(0,equalPos);
    r = line.substr(equalPos+1);

    StringUtil::Trim(l);
    StringUtil::Trim(r);

    //Skip empty keys or values
    if (l.size() == 0 || r.size() == 0)
    {
        //Line with nothing but equals sign
        return false;
    }

    //Ignore second definitions of configuration values
    if (configurationsByName.find(l) == configurationsByName.end())
    {

        configurationsByName[l] = configurations.size();
        configurations.push_back({l, r});
        return true;
    }
    return false;
}

void ConfigGroup::writeToFile(GBVFS::FileWriter* file)
{
    if (!file)
        return;

    for (auto& p : configurations)
    {
        file->writeLine(p.first+" = "+p.second);
    }
}

void ConfigGroup::parseFromFile(GBVFS::File* file)
{
    if (!file)
        return;
    
    while (!file->isEOF())
    {
        parseLine(file->readLine());
    } 
}

ConfigGroup* ConfigCollection::createGroup(const char* group)
{
    if (!group)
        return nullptr;

    ConfigGroup* config;
    auto it = configGroups.find(group);
    if (it == configGroups.end())
    {
        auto uptr = std::make_unique<ConfigGroup>();
        config = uptr.get();
        configGroups[group] = std::move(uptr);
    }
    else
    {
        Log << "ConfigGroup '" << group << "'' already exists" << Trace(CHash("Warning"));
        return nullptr;
    }
    return config;
}

void ConfigCollection::addFromFileToGroup(GBVFS::File* file, const char* group)
{
    if (!file || !group)
        return;

    ConfigGroup* config;
    auto it = configGroups.find(group);
    if (it == configGroups.end())
    {
        config = createGroup(group);
    }
    else
    {
        config = it->second.get();
    }
    if (config)
        config->parseFromFile(file);
    else
        Log << "Failed to load ConfigGroup " << group << " from file" << Trace(CHash("Warning"));
}

void ConfigCollection::writeGroupToFile(GBVFS::FileWriter* file, const char* group)
{
    if (!file || ! group)
        return;

    auto it = configGroups.find(group);
    if (it == configGroups.end())
    {
        throw std::runtime_error(std::string("ConfigGroup ")+group+" doesn't exist");
    }
    it->second->writeToFile(file);
}

bool ConfigCollection::getConfiguration(const char* group, const char* key, std::string& out)
{

    auto it = configGroups.find(group);
    if (it == configGroups.end())
        return false;

    auto it2 = it->second->configurationsByName.find(key);
    if (it2 == it->second->configurationsByName.end())
        return false;

    out = it->second->configurations[it2->second].second;
    return true;
}
