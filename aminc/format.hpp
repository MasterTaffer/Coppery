#pragma once
#include <string>
#include <vector>

#include <sstream>


//Note that the file documentation is made with doxygen in mind:
//Doxygen replaces "%_" with "_". That is why in the Variations
//section the "%_" and "%_4$" are prefixed with one percent sign.

/*! \file format.hpp
 *  \brief Format string library
 * 
 * 
 * **Format string syntax:**
 * 
 * Every character is encoded by themselves EXPECT for `%`: it is encoded
 * by `%%`
 * 
 * The format argument specifiers are signified by single % symbols
 * 
 * Variations:
 * 
 *  - `%`	argument specifier with automatic incrementing indexes
 *  - `%%_`	argument specifier that won't be printed (must be the first character after `%`)
 *  - `%6`	argument specifier with explicit index specifier
 *  - `%$`	argument specifier with explicit end symbols
 *  - `%%_4$`	combinations of above
 * 
 * The format indexes start from '0': if no indexes are specified they are
 * automatically assigned starting from 0 and incrementing by 1 after that
 * 
 * A format string `"Hey %! It's % %!"` will have 3 format arguments at indexes 0, 1 and 2
 * 
 * `The %2 is %0%% off and now costs %1$$!` also has 3 format arguments (indexes 2, 0, 1 from left to right)
 * if the format parameters `[25, 12, "golden egg"]` is applied to the above example, the output would be
 * `The golden egg is 25% off and now costs 12$!`
 * 
 * Note the use of both `%%` and format argument terminator `$` in the above example. The format argument
 * terminator is required when the format argument is immediately followed by valid argument specifier 
 * characters.
 * 
 */

//! Class representing a compiled format string
class FormatString
{
	struct Element
	{
		size_t base;
		size_t index = 0;
		bool ignore = false;
	};
	std::string base;
	std::vector<Element> elements;
	size_t requiredElements = 0;

	std::string err;
	void setError(const std::string& error);
	
	friend class FormatStringFormatter;
public:
	
    //! Gets how many format arguments are required by this format string
	size_t getRequiredArgumentCount()
	{
		return requiredElements;
	}
	
	//! If an error occured returns a string representation of that error
	std::string getError()
	{
		return err;
	}
	
	//! Clears an resets the state of the format string to empty string
	void clear();
    
    /*! \brief Compiles a format string
     * 
     * See the format.hpp file documentation on the structure of 
     * format strings.
     * 
     * If the method fails, an error string can be retrieved with
     * getError().
     * 
     * \return true on success, false on error
     */
	bool compile(const char* str);
};

//! Class used for formatting a compiled format string
class FormatStringFormatter
{
	std::vector<std::pair<bool, std::string>> elements;
    void resizeIfGreater(size_t newSize)
    {
        if (newSize > elements.size())
        {   
            elements.resize(newSize, {false, ""});
        }
    }
    int uninitializedElements = 0;
        
public:
    
    /*! \brief Clear an argument at index.
     * 
     * Clears the argument at specified index.
     * Will also resize the argument list of the formatter to fit the
     * specified index.
     * 
     * One may resize the argument list to correct size using this method:
     *
     *```
     *  clear(lastIndex);
     *```
     * 
     * however, this will only work if the current size is smaller or
     * equal to desired size.
     */
    
    void clear(size_t index)
    {
        resizeIfGreater(index + 1);
        elements[index] = {false, ""};
    }
    
    
    //! \copydoc set(const T el, size_t index)
    void set(const std::string& el, size_t index)
    {
        resizeIfGreater(index + 1);
        elements[index] = {true, el};
    }
    
    //! \copydoc set(const T el, size_t index)
    void set(const char* el, size_t index)
    {
        resizeIfGreater(index + 1);
        elements[index] = {true, std::string(el)};
    }
    
    /*! \brief Set a format string argument
     * 
     * Sets a single argument at a specified index. Will resize the argument
     * list to fit the specified index.
     * 
     * \param el the argument to insert
     * \param index the target index on the argument list
     */
	template <typename T>
	void set(const T el, size_t index)
    {
		std::stringstream ss;
		ss << el;
        set(ss.str(), index);
    }
	
    
    //! \copydoc add(const T el)
	void add(const std::string& el)
	{
		elements.push_back({true, el});
	}
	
	//! \copydoc add(const T el)
	void add(const char* el)
	{
		std::string ss = el;
		elements.push_back({true, ss});
	}
	
	/*! \brief Adds a new format argument.
     * 
     * Increments the size of the argument list by one and sets the last
     * argument. This method will always increase the size of the argument
     * list:
     *
     *```
     *  set("at index 4", 4);
     *  set("at index 2", 2);
     *  add("at index 5");
     *```
     * 
     * will result in argument list of size 6. 
     * 
     * \param el the argument to add
     */
	template <typename T>
	void add(const T el)
	{
		std::stringstream ss;
		ss << el;
		elements.push_back({true, ss.str()});
	}
	
	/*! \brief Formats a string
     * 
     * The argument list size must exactly match the FormatString's required
     * arguments. Otherwise an error will occur. "Missing" arguments do not
     * cause any errors: they are replaced with blanks.
     * 
     * \param fs the format string
     * \param out the output string
     * \return 0 on success, negative values on error
     */
	
	int format(const FormatString& fs, std::string& out)
	{
        
		if (fs.requiredElements != elements.size())
			return -1;
		out.clear();
		
		size_t pos = 0;
		for (auto & el : fs.elements)
		{
			out.append(fs.base.c_str() + pos, el.base - pos);
			pos = el.base;
			if (el.ignore)
				continue;
			out.append(elements[el.index].second);
		}
		out.append(fs.base.c_str() + pos);
		return 0;
	}
	
	/*! \brief Formats a string using parent formatter.
     * 
     * The parent formatter is used to fetch the missing arguments
     * 
     * The argument list of the parent must exactly match the FormatString's
     * required arguments. The base formatters argument list's size doesn't
     * matter, as the missing arguments are fetched from the parent formatter.
     * 
     * Argument is considered missing if the argument was not added via the
     * "add" or "set" methods. The "clear" method can remove a previously
     * "present" argument and set its state to missing.
     * 
     * When resizing the argument list via the "set" or "clear" methods,
     * the indexes not explicitly set will be in "missing" state. 
     * 
     * \param fs the format string
     * \param parent the parent formatter
     * \param out the output string
     * \return 0 on success, negative values on error
     */
    
	int formatWithParent(const FormatString& fs, const FormatStringFormatter& parent, std::string& out)
	{
        if (fs.requiredElements != parent.elements.size())
			return -2;
        
		out.clear();
		
		size_t pos = 0;
		for (auto & el : fs.elements)
		{
			out.append(fs.base.c_str() + pos, el.base - pos);
			pos = el.base;
			if (el.ignore)
				continue;
            
            if (el.index < elements.size() && elements[el.index].first)
                out.append(elements[el.index].second);
            else
                out.append(parent.elements[el.index].second);
		}
		out.append(fs.base.c_str() + pos);
		return 0;
	}
};


#ifdef FORMATSTRING_IMPLEMENTATION
#include <cctype>

void FormatString::clear()
{
	elements.clear();
	base.clear();
	requiredElements = 0;
	err = "";
}

bool FormatString::compile(const char* str)
{
	clear();
	size_t start = 0;
	int nextIndex = 0;
	size_t i;
	int greatestIndex = -1;

	for (i = 0; str[i]; i++)
	{

		auto readNum = [&](bool& gotNum)
		{
			int num = 0;
			gotNum = false;
			while (std::isdigit(str[i]))
			{
				const char numzeroascii = 0x30;
				gotNum = true;
				num *= 10; 
				num += (str[i] - numzeroascii);
				i++;
			}
			return num;
		};

		if (str[i] == '%')
		{

			size_t cnt = (i - start);
			if (cnt > 0)
				base.append(str + start, cnt);

			i++;
			if (str[i] == '%')
			{
				base.append("%%",1);
				start = i + 1;
				continue;
			}


			Element el;
			el.base = base.size();

			if (str[i] == 'i')
			{
				el.ignore = true;
				++i;
			}

			

			bool gotNum = false;
			int curIndex = nextIndex;
			int idx = readNum(gotNum);
			if (gotNum)
			{
				if (idx < 0)
				{
					setError("Invalid index");
					return false;
				}
				/*
				if (str[i] != '$')
				{
					setError("Expected index specifier sign '$'");
					return false;
				}
				
				i++;
				*/

				curIndex = idx;
				
			}
			else
			{
				curIndex = nextIndex; 
			}

			nextIndex = curIndex + 1;
			el.index = curIndex;

			if (curIndex > greatestIndex)
				greatestIndex = curIndex;
			
			if (str[i] == '$')
				++i; 
			
			elements.push_back(el);
			
			start = i;
			--i;
		}
	}
	size_t cnt = (i - start);
	if (cnt > 0)
		base.append(str + start, cnt);
	
	requiredElements = greatestIndex + 1;
	
	return true;
}

void FormatString::setError(const std::string& error)
{
	err = error;
}


#endif
