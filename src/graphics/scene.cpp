#include "scene.hpp"

#include "glUtilities.hpp"

#include "graphics.hpp"

#include "graphicsData.hpp"

#include "oGL.hpp"


bool SceneBranch::traverse()
{
    for (auto it = nodes.begin(); it != nodes.end();)
    {
        if ((*it).second->traverse())
        {
            auto it2 = it;
            it++;

            (*it2).second->clear();
            delete ((*it2).second);

            nodes.erase(it2);
        }
        else
            it++;
    }
    return false;
}

SceneNodeBase* SceneBranch::getBranch(intptr_t i)
{
    auto it = nodes.find(i);
    if (it == nodes.end())
        return createNode(i);
    return (*it).second;
}

void SceneBranch::clear()
{
    for (auto it = nodes.begin(); it != nodes.end(); it++)
    {
        (*it).second->clear();
        delete ((*it).second);
    }
    nodes.clear();
    return;
}

bool SceneBranch::insertNode(SceneNodeBase* a)
{
    intptr_t i = a->getValue();
    nodes.insert({i, a});
    return true;
}

SceneMasterBranch::SceneMasterBranch(Graphics* g)
    : SceneBranch(g)
{
}

SceneMasterBranch::~SceneMasterBranch()
{
    if (perfQuery.size() > 0)
    {
        #ifndef COPPERY_HEADLESS
        glDeleteQueries(perfQuery.size(), perfQuery.data());
        #endif
    }
    perfQuery.clear();
}

bool SceneMasterBranch::insertNode(SceneNodeBase* node)
{
    auto status = SceneBranch::insertNode(node);
    if (status)
    {
        unsigned int val = 0;
        #ifndef COPPERY_HEADLESS
        glGenQueries(1, &val);
        #endif
        perfQuery.push_back(val);
        perfQueryKeys.push_back(0);
    }
    return status;
}
bool SceneMasterBranch::traverse()
{
    size_t index = 0;
    for (auto it = nodes.begin(); it != nodes.end();)
    {
        perfQueryKeys[index] = it->first;
        
        #ifndef COPPERY_HEADLESS
        glBeginQuery(GL_TIME_ELAPSED, perfQuery[index]);
        #endif
        index += 1;
        if ((*it).second->traverse())
        {
            auto it2 = it;
            it++;

            (*it2).second->clear();
            delete ((*it2).second);

            nodes.erase(it2);
        }
        else
            it++;
        #ifndef COPPERY_HEADLESS
        glEndQuery(GL_TIME_ELAPSED);
        #endif
    }
    return false;
}
std::pair<unsigned int, intptr_t> SceneMasterBranch::getPerformanceQuery(size_t i)
{
    if (i >= perfQuery.size())
        return {0,0};
    return {perfQuery[i], perfQueryKeys[i]};
}
