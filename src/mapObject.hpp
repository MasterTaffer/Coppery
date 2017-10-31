#pragma once
#include <vector>
#include <string>
#include <utility>
#include "reference.hpp"

class MapObject
{
    MixinReferenceCounted
public:
    int x;
    int y;
    int id;
    std::string type;
    std::string name;
    std::vector<std::pair<std::string,std::string> > properties;
    
    size_t getPropertyCount()
    {
        return properties.size();
    }
    
    std::string getPropertyName(size_t index)
    {
        if (index >= properties.size())
            return "";
        return properties[index].first;
    }
    
    std::string getPropertyValue(size_t index)
    {
        if (index >= properties.size())
            return "";
        return properties[index].second;
    }
};
