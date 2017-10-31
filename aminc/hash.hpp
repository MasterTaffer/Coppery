#pragma once
#include <cstdint>
#include <string>


#ifndef HASH_BYTES

/*! \def HASH_BYTES
 *  \brief The size of hash used application wide
 */

#define HASH_BYTES 8
#endif

/*! \brief Parameters used for FNV-1a hashing algorithm
 * 
 * HashParameters will only contain valid information for valid hash sizes
 * such as HashParameters<4> and HashParameters<8>.
 * 
 * \tparam size of hash used in bytes
 */
template<int T>
struct HashParameters
{
    //! The arithmetic type used to calculate and store the hash value
    typedef struct {} HashUInt;
    
    //! The value of FNV-1a offset basis for the hash size
    constexpr static const HashUInt Base {};
    //! The value of FNV-1a prime for the hash size
    constexpr static const HashUInt Prime {};
};

template<>
struct HashParameters<4>
{
    typedef uint32_t HashUInt;
    constexpr static const HashUInt Base = 2166136261;
    constexpr static const HashUInt Prime = 16777619;
};

template<>
struct HashParameters<8>
{
    typedef uint64_t HashUInt;
    constexpr static const HashUInt Base = 14695981039346656037UL;
    constexpr static const HashUInt Prime = 1099511628211UL;
};

//! Simple character string hashing algorithm using FNV-1a
class Hash
{
public:
    //! size of single hash
    constexpr static const int Bytes = HASH_BYTES;
    
    //! The FNV-1a parameters to use
    typedef HashParameters<Bytes> Parameters;
    typedef Parameters::HashUInt HashUInt;
    constexpr static const HashUInt Base = Parameters::Base;
    constexpr static const HashUInt Prime = Parameters::Prime;
    
    

    //! Hash a null terminated input string using FNV-1a
    static HashUInt constexpr const_hash(char const *input)
    {
        if (input == nullptr)
            return 0;
        HashUInt h = Base;
        while (*input)
        {
            h ^= static_cast<HashUInt>(*input);
            h *= Prime;
            ++input;
        }
        return h;
    };

    //! Run-time hashing from std::string
    static HashUInt dyn_hash_string(const std::string& input)
    {
        return dyn_hash(input.c_str());
    };


    //! Run-time hashing from char*
    static HashUInt dyn_hash(const char *input)
    {
        return const_hash(input);
    }

    //! Constructor from cstring
    constexpr explicit Hash(const char* str) : value(const_hash(str))
    {

    };

    //! Constructor directly from the hash value
    constexpr explicit Hash(HashUInt v) : value(v)
    {
    };

    //! Constructor from std string
    explicit Hash(const std::string& str) : value(const_hash(str.c_str()))
    {
    };

    //! Returns the hash
    operator HashUInt() const
    {
        return value;
    };

    //! Comparison
    bool operator <(const Hash& b) const
    {
        return value<b.value;
    };

    //! Comparison
    bool operator ==(const Hash& b) const
    {
        return value==b.value;
    };

    //! Comparison
    bool operator >(const Hash& b) const
    {
        return value>b.value;
    };

    //! Hash
    HashUInt value;
};

//! Hash literal constructor

constexpr Hash operator "" _hash(const char* str, size_t a)
{
    return Hash(str);
}

/*! \def CHash(str)
 *  \brief A guaranteed compile time evaluated hash macro
 */

#define CHash(str) Hash(std::integral_constant<Hash::HashUInt, Hash::const_hash(str)>::value)


namespace std {
    
    //! std::hash template specialization for Hash
    template <>
    struct hash<Hash> {
    public:
        //! The hashing function for Hash (identity)
        size_t operator()(const Hash &x) const {
            return std::hash<typename Hash::HashUInt>()(x.value);
        }
    };
}
