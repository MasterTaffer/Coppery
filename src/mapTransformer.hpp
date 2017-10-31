#pragma once

/*! \brief Performs transformation between two maps.
 * 
 * Generates transformation array for map of type MapFrom<Key, Integer> to
 * MapTo<Key, Value>, so that each Integer key has Value. This transformation
 * can be then queried using data[Integer].
 * 
 */
template<class MapFrom, class MapTo, class Value, typename Integer>
class MapTransformation
{
public:
    //! Indicates if the transformation is invalid.
    bool invalid;
    
    //! Indicates if some transformation pairs are incomplete.
    bool incomplete;
    
    //! Indicates if there was (discarded) duplicate transformation pairs.
    bool duplicates;
    
    //! Total size of \a data
    size_t size;
    
    //! The transformation array
    Value* data;

    //! Default constructor
    MapTransformation()
    {
        invalid = true;
        incomplete = false;
        duplicates = false;
        size = 0;
        data = nullptr;
    }
    
    //! Move constructor
    MapTransformation(MapTransformation&& other)
    {
        invalid = other.invalid;
        incomplete = other.incomplete;
        duplicates = other.duplicates;
        size = other.size;
        data = other.data;
        
        other.invalid = true;
        other.incomplete = false;
        other.duplicates = false;
        other.size = 0;
        other.data = nullptr;
    }
    
    //! Move assignment operator
    MapTransformation& operator = (MapTransformation&& other)
    {
        invalid = other.invalid;
        incomplete = other.incomplete;
        duplicates = other.duplicates;
        size = other.size;
        data = other.data;
        
        other.invalid = true;
        other.incomplete = false;
        other.duplicates = false;
        other.size = 0;
        other.data = nullptr;
        return *this;
    }
        
    
    /*! \brief Constructor to generate map transformation
     * 
     * In the input map \p a the Integer values shall not be greater than size
     * property of \p a. Otherwise the \a invalid flag will be set. The 
     * \a incomplete flag will be set if \p a has keys that \p b doesn't have.
     * \a duplicates will be set if \p a has multiple cases of the same
     * Integer.
     * 
     * The transformation array will have exactly a.size + 1 elements, with 0
     * being a special case. Typically one would include all Integer values
     * in the range [1, a.size()] in \p a, and this is the intended purpose.
     * The Key of these Key-Integer pairs should also be the Key of single
     * Key-Value pair in \p b.
     * 
     * \p def will be used to initialize all transformation elements not
     * defined in \p a or \p b.
     */
    MapTransformation(const MapFrom& a, const MapTo& b, const Value& def)
    {
        size = a.size() + 1;
        data = new Value[size];
        for (size_t i = 0; i < size; i++)
        {
            data[i] = def;
        }
                
        bool* duplicateDetector = new bool[size]();
        
        invalid = false;
        incomplete = false;
        duplicates = false;
        
        for (auto& pair : a)
        {
            if (size_t(pair.second) >= size || pair.second < 0)
            {
                invalid = true;
                continue;
            }
        
            if (duplicateDetector[pair.second])
            {
                duplicates = true;
                continue;
            }
            
            duplicateDetector[pair.second] = true;
            
            auto it = b.find(pair.first);
            
            if (it == b.end())
            {
                incomplete = true;
            }
            else
            {
                data[pair.second] = it->second;
            }
        }
        
        delete[] duplicateDetector;
    }
    
    //! Destructor
    ~MapTransformation()
    {
        delete[] data;
    }
    
};
