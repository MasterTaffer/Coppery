#pragma once
#include <vector>
#include <utility>
#include <unordered_map>
#include "hash.hpp"

class Shader;

class UniformMap
{
    Shader* forShader = nullptr;
    //pair of index to vector below & uniform value
    std::unordered_map<Hash, std::pair<int, float>> name_uniform1fs;
    //shader uniform name & uniform value
    std::vector<std::pair<int, float>> uniform1fs;


    int findUniform1f(Hash h, float f);
public:


    UniformMap()
    {
    }    

    UniformMap(Shader* s)
    {
        forShader = s;
    }

    UniformMap(const UniformMap& other, Shader* s)
    {
        forShader = s;

        for (auto& p : other.name_uniform1fs)
            setUniform1f(p.first, p.second.second);
    }

    void setUniform1f(Hash h, float f);
    
    void apply();
};
