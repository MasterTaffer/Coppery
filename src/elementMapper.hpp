
/*! \def ELEMENTMAPPER_KEY_TYPE
 *  \brief The key type used in ElementMapper
 */
#define ELEMENTMAPPER_KEY_TYPE Hash

#pragma once
#include <typeinfo>
#include "log.hpp"
#include "hash.hpp"

#include <vector>
#include <unordered_map>

/*! \brief Structure used for asset loading and fetching
 * 
 * Will provide functionalities for storing and getting different types of
 * assets. Different asset containers may overload initializeElement and
 * deInitializeElement methods to provide asset construction and destruction
 * utilities.
 * 
 * The addElement will construct new elements using the default constructor
 * of type T. 
 * 
 * \tparam T the type of asset stored
 */
template <typename T>
class ElementMapper
{

    std::unordered_map<ELEMENTMAPPER_KEY_TYPE, unsigned int> NameMap;

protected:

    //! Array of all elements
    std::vector<T*> ElementList;

    //! Number of new elements (that are unitialized)
    size_t newElements = 0;

public:

    //! Destructor which deinitializes and destructs all elements as well
    virtual ~ElementMapper()
    {
        deInit();
        clearElements();
    }

    /*! \brief Fetches an element by name
     * 
     * Will print a warning on the log if the element doesn't exist.
     * 
     * \return the fetched element, or nullptr on failure
     */
    T* getElement(ELEMENTMAPPER_KEY_TYPE name)
    {
        
        auto d = NameMap.find(name);
        if (d == NameMap.end())
        {
            Log << "Failed to find " << typeid(T).name() << " " << VariableData(name) << Trace(CHash("Warning"));
            return nullptr;
        }
        return ElementList[(*d).second];
    }

    /*! \brief Searches for an element by name
     * 
     * Unlike getElement this will not print a warning message on failure.
     *  
     * \return the fetched element, or nullptr on failure
     */
    
    T* tryGetElement(ELEMENTMAPPER_KEY_TYPE name)
    {
        auto d = NameMap.find(name);
        if (d == NameMap.end())
        {
            return nullptr;
        }
        return ElementList[(*d).second];
    }

    /*! \brief Constructs and adds a new element
     * 
     * Will print a warning message if an element with the same name
     * already exists.
     * 
     * \return the new created element
     */

    T* addElement(ELEMENTMAPPER_KEY_TYPE name)
    {
        if (NameMap.find(name) != NameMap.end())
        {
            //In case of collisions
            //Add the new element to the backing data vector
            //but never add it to the key map

            Log << "Failed to insert " << typeid(T).name() << " " << VariableData(name) ": duplicate names" << Trace(CHash("Warning"));
        }
        else
            NameMap[name] = ElementList.size();

        T* d = new T();
        ElementList.push_back(d);
        newElements += 1;
        return d;
    }

    //! Initializes all new elements
    void init()
    {
        if (newElements == 0)
            return;

        for (auto it = ElementList.end() - newElements; it != ElementList.end(); ++it)
        {
            T* e = *it;
            initializeElement(e);
        }
        newElements = 0;
    }

    //! Deinitializes all elements
    void deInit()
    {
        //There's no need to deinitialize an element that was never initialized
        auto endit = ElementList.end() - newElements;
        for (auto it = ElementList.begin(); it != endit; ++it)
        {
            T* e = *it;
            deInitializeElement(e);
        }

        //Mark all elements as uninitialized elements
        newElements = ElementList.size();
    }

protected:

    void clearElements()
    {
        for (T* e: ElementList)
        {
            delete e;
        }
        NameMap.clear();
        ElementList.clear();
        newElements = 0;
    }

    //! Initializes a single element
    virtual void initializeElement(T*){};

    //! Deinitializes a single element
    virtual void deInitializeElement(T*){};
};
