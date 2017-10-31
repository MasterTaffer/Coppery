#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>

//! Class for parsing command line arguments
class ArgumentParser
{
public:

    typedef std::unordered_map<std::string, std::string> argstype;

    //! Argument keys and parameters
    std::unordered_map<std::string, std::string> arguments;
    

    //! Try parse an argument to a value
    template <typename T>
    bool parseToValue(argstype::iterator& it, T& val)
    {
        if (it->second.size() == 0)
            return false;
        const std::string& ref = it->second;
        std::stringstream sst;
        sst << ref;
        sst >> val;
        if (sst.fail())
            return false;
        return true;
    }
    //! Constructor from list of arguments
    ArgumentParser(int argc, const char** argv)
    {
        std::vector<std::vector<std::string>> aarr;
        for (int i = 0; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {

                aarr.push_back(std::vector<std::string>());
                aarr.rbegin()->push_back(argv[i]);
            }
            else
            {
                if (aarr.size() > 0)
                    aarr.rbegin()->push_back(argv[i]);
                //else -- Handle error case
            }
        }

        for (auto& v : aarr)
        {
            if (v.size() == 0)
                continue;
            auto it = arguments.find(v[0]);
            if (it == arguments.end())
            {
                arguments[v[0]] = "";
            }
            std::string& ref = arguments[v[0]];

            for (auto it = v.begin() + 1; it != v.end(); it++)
            {
                if (ref.size() == 0)
                    ref = *it;
                else
                    ref = ref + " " + *it;
            }
        }
    }

    //! Get if argument flag is present
    bool getArgument(const std::string& shortForm, const std::string& longForm)
    {
        if (shortForm.size() > 0)
        {
            auto it = arguments.find(shortForm);
            if (it != arguments.end())
                return true;
        }

        if (longForm.size() > 0)
        {
            auto it2 = arguments.find(longForm);
            return (it2 != arguments.end());
        }

        return false;
    }

    /*! \brief Get argument value
     *
     * \param shortForm the short form of the argument key
     * \param longForm the long form of the argument key
     * \param output output value
     * \return true if the value was read successfully
     */

    template <typename T>
    bool getArgument(const std::string& shortForm, const std::string& longForm, T& output)
    {
        if (shortForm.size() > 0)
        {
            auto it = arguments.find(shortForm);
            if (it != arguments.end())
            {
                parseToValue(it, output);
            }
        }

        if (longForm.size() > 0)
        {
            auto it = arguments.find(longForm);
            if (it != arguments.end())
            {
                parseToValue(it, output);
            }
        }
        return false;
    }

    //! Return the full parameter string
    bool getArgumentFullString(const std::string& shortForm, const std::string& longForm, std::string& output)
    {
        if (shortForm.size() > 0)
        {
            auto it = arguments.find(shortForm);
            if (it != arguments.end())
            {
                output = it->second;
            }
        }

        if (longForm.size() > 0)
        {
            auto it = arguments.find(longForm);
            if (it != arguments.end())
            {
                output = it->second;
            }
        }
        return false;
    }
};

