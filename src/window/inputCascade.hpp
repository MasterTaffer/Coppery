#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "hash.hpp"
#include "input.hpp"


class InputStyle;

/*! \brief Single layer of input in the InputCascade
 * 
 * Represents a single "level" of input, for eaxample UI input, game input
 * etc..
 */
class InputLayer
{
    std::vector<Input> inputs;
    friend class InputCascade;

public:
    InputLayer(unsigned int index) : index(index) 
    {

    }

    bool active = true;
    bool stopFlow = false;
    const unsigned int index = 0;
};

/*! \brief Class handling the priority and propagation of input
 * 
 * This class handles different levels of input and whether the input
 * propagates to a lower level. For example when a text input box is open
 * keyboard inputs should not be handled anywhere else.
 * 
 * The levels of propagation are represented by InputLayer instances. These
 * instances control whether the specific layer of input will stop the
 * propagation of input events (e.g when the text box is open).
 * 
 * The input always flows from smaller index to the greater index. If one
 * wishes to have 3 layers of inputs, one should call addLayer 3 times.
 */
class InputCascade
{
    size_t inputCount = 0;
    Input nullInput;
    std::vector<std::unique_ptr<InputLayer>> inputLayers;
    std::unordered_map<Hash, size_t> inputIndices;
public:
    
    /*! \brief Method to get a single layer of input
     * 
     * Returns the InputLayer specified by the index.
     * 
     * \param inputLayer index of the InputLayer
     * \return The InputLayer requested or a nullptr on failure
     */
    InputLayer* getInputLayer(unsigned int inputLayer);
    
    /*! \brief Method to get a single Input from a specific inputLayer
     * 
     * Returns the Input associated with the Hash h from the InputLayer
     * specified by the index \p inputLayer. If the input is not found a null
     * dummy input is returned and a warning is logged.
     * 
     * \param h the hash representing the input
     * \param inputLayer index of the InputLayer
     * \return The Input requested or a null dummy input on failure
     */
    
    const Input& getInput(Hash h, unsigned int inputLayer);
    
    /*! \brief Adds a new layer of input
     * 
     * \return The new input layer index
     */
    
    unsigned int addLayer();

    /*! \brief Adds new Input instances from an array of InputStyle pointers
     * 
     * The inputIndex of the InputStyle instances are set to their correct
     * values by this method. 
     */
    void createInputs(const std::vector<InputStyle*> inputs);

    /*! \brief Applies a list of input events
     * 
     * Use to create new input events. The input vector elements have two
     * components: the input index (from InputStyle inputIndex) and the
     * floating point input value (typically ranging from -1.0 to 1.0).
     * 
     * \param input the list of input events to be added
     */
    void flowInput(const std::vector<std::pair<int, float>>& input);
    
    /*! \brief Updates all inputs 
     * 
     * This method should be called once after adding the input events via
     * flowInput calls. The method effectively "steps" the input changing
     * the states of Input instances from "pressed" to "held" and such.
     */
    void update();
};
