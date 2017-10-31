#include "log.hpp"
#include "inputCascade.hpp"
#include "keys.hpp"

const Input& InputCascade::getInput(Hash input, unsigned int layer)
{
    if (inputLayers.size() <= layer)
    {
        Log << "Invalid input layer index " << layer << Trace(CHash("Warning"));
        return nullInput;
    }
    
    auto it = inputIndices.find(input);
    if (it == inputIndices.end())
    {
        Log << "Invalid input " << input << Trace(CHash("InputWarning"));
        return nullInput;
    }

    //Should throw on failure:
    //	inputLayer's inputs should always have the correct index
    return inputLayers[layer]->inputs.at(it->second);
}


InputLayer* InputCascade::getInputLayer(unsigned int inputLayer)
{
    if (inputLayers.size() <= inputLayer)
    {
        Log << "Invalid input layer index " << inputLayer << Trace(CHash("Warning"));
        return nullptr;
    }
    return inputLayers[inputLayer].get();
    
}

unsigned int InputCascade::addLayer()
{
    unsigned int idx = inputLayers.size();
    inputLayers.push_back(std::make_unique<InputLayer>(idx));
    inputLayers.rbegin()->get()->inputs.resize(inputCount);
    return idx;
}

void InputCascade::createInputs(const std::vector<InputStyle*> inputs)
{
    for (auto& input : inputs)
    {
        auto hash = Hash(input->inputName);
        auto it = inputIndices.find(hash);
        if (it != inputIndices.end())
        {
            input->inputIndex = it->second;
            continue; //Input already registered: move on
        }

        inputIndices[hash] = inputCount;
        input->inputIndex = inputCount;
        inputCount += 1;

    }

    for (auto& layers : inputLayers)
    {
        layers->inputs.resize(inputCount);
    }

}

void InputCascade::flowInput(const std::vector<std::pair<int, float>>& input)
{

    for (auto& inputPair : input)
    {
        if (inputPair.first < 0 || static_cast<size_t>(inputPair.first) >= inputCount)
        {
            Log << "Invalid input index in flow " << inputPair.first << Trace(CHash("InputWarning"));
            continue;
        }
    
        for (auto& p : inputLayers)
        {
            InputLayer* il = p.get();
            if (il->stopFlow)
                break;

            if (!il->active)
                continue;

            il->inputs.at(inputPair.first).updateAnalog(inputPair.second);
        }
    }
}

void InputCascade::update()
{
    for (auto& p : inputLayers)
    {
        for (auto& ip : p->inputs)
        {
            ip.update();
        }
    }
}
