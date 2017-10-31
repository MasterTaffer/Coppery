#pragma once
#include <string>

namespace StringUtil
{
    //! Trims whitespace from the beginning and the end of the input string
    void Trim(std::string& str);

    /*! Splits the strings at the first occurence of c

        Returns the part after the first occurence or an empty
        string if no occurence was found.
        str will also be modified to contain the part before the 
        first occurence, or the full string if no occurence was found.
    */
    std::string SplitFirst(std::string& str, char c);
}
