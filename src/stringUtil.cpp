#include "stringUtil.hpp"
#include <algorithm>

namespace StringUtil
{

    /*! \brief Trims whitespace from both ends of the string
     * 
     * \param str input and output string to be trimmed
     */
    void Trim(std::string& str)
    {
        str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), ::isspace));
        str.erase(std::find_if_not(str.rbegin(), str.rend(), ::isspace).base(), str.end());
    }

    /*! \brief Splits the string at the first occurence of a character
     * 
     * \param str input to be splitted and output for the first half of the string
     * \param c splitter character
     * 
     * \return The second half of the split string
     */
    
    std::string SplitFirst(std::string& str, char c)
    {
        auto pos = str.find(c);
        if (pos == str.npos)
            return "";

        std::string ret = str.substr(pos + 1);
        str.erase(str.begin() + pos, str.end());
        return ret;
    }
}
