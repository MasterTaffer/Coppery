#include "log.hpp"
#include "keyConfig.hpp"
#include "fileOperations.hpp"
#include "fileWriterStreambuf.hpp"
#include "stringUtil.hpp"

#include <sstream>
#include <algorithm>

KeyConfig::KeyConfig()
{
    auto file = GetFileStream("static/keyaliases");
    
    if (file)
    {
        
        while (!file->isEOF())
        {
            auto buffer = file->readLine();
            
            std::string spl = StringUtil::SplitFirst(buffer, '=');
            
            StringUtil::Trim(spl);
            if (spl.size() == 0)
                continue;
            
            StringUtil::Trim(buffer);
            
            if (buffer.size() == 0)
                continue;
            
            std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);
            
            std::stringstream ss;
            ss << spl;
            int val;
            ss >> val;

            keyAliases[buffer] = val;
            keyAliasesAssoc[val] = buffer;
        }
    }
}

void KeyConfig::addValue(const std::string& bstr, const std::string& name)
{
    
    bool invert = false;
    bool repeat = false;
    
    std::string original = bstr;
    
    auto parseFlags = [&](std::string& rest)
    {
        StringUtil::Trim(rest);
        while (rest.size() > 0)
        {
            std::string more = StringUtil::SplitFirst(rest, ' ');
            std::transform(rest.begin(), rest.end(), rest.begin(), ::tolower);
            if (rest == "inverted")
                invert = true;
            else if (rest == "enablerepeat")
                repeat = true;
            else
                Log << "Unknown input flag \"" << rest << "\" at \"" << original << "\"" << Trace(CHash("Warning"));
            
            StringUtil::Trim(rest);
            std::swap(rest, more);
        }
    };
    
    std::string basestr = bstr;
    
    std::string aftername = StringUtil::SplitFirst(basestr, ' ');
    StringUtil::Trim(aftername);
    
    if (aftername.size() == 0)
    {
        Log << "Invalid input definition \"" << original << "\"" << Trace(CHash("Warning"));
        return;
    }
    
    std::string rest = StringUtil::SplitFirst(aftername, ' ');
    StringUtil::Trim(rest);
    
    int num = -1;
    
    std::transform(basestr.begin(), basestr.end(), basestr.begin(), ::tolower);
    
    if (basestr == "key")
    {
        std::transform(aftername.begin(), aftername.end(), aftername.begin(), ::tolower);
        
        auto it = keyAliases.find(aftername);
        if (it != keyAliases.end())
        {
            num = it->second;
        }
    }
    
    if (num <= -1)
    {
        std::stringstream ss(aftername);
        ss >> num;
    
        if (ss.fail())
        {
            Log << "Invalid input id at \"" << original << "\"" << Trace(CHash("Warning"));
            return;
        }
    }
    
    
    
    if (basestr == "joy")
    {
        //rest <- button | axis
        
        std::string rest2 = StringUtil::SplitFirst(rest, ' ');
        StringUtil::Trim(rest);
        
        //rest2 <- button or axis id
        //rest3 <- rest of the string
        std::string rest3= StringUtil::SplitFirst(rest2, ' ');
        StringUtil::Trim(rest2);
        
        
        std::stringstream ss(rest2);
        
        int joynum = 0;
        ss >> joynum;
    
        if (ss.fail())
        {
            Log << "Invalid input joystick id at \"" << original << "\"" << Trace(CHash("Warning"));
            return;
        }
        
        parseFlags(rest3);
        
        std::transform(rest.begin(), rest.end(), rest.begin(), ::tolower);
        if (rest == "button")
        {
            JoyButtonInputStyle mi;
            mi.inputName = name;
            mi.button = joynum;
            mi.joy = num;
            mi.reversed = invert;
            jbuttons.push_back(mi);
            return;
        }
        else if (rest == "axis")
        {
            
            JoyAxisInputStyle mi;
            mi.inputName = name;
            mi.axis = joynum;
            mi.joy = num;
            mi.reversed = invert;
            axes.push_back(mi);
            return;
        }
        
        Log << "Invalid joystick input type at \"" << original << "\"" << Trace(CHash("Warning"));
        return;
    }
    else if (basestr == "key")
    {
        parseFlags(rest);
        
        KeyInputStyle mi;
        mi.inputName = name;
        mi.key = num;
        mi.reversed = invert;
        mi.repeat = repeat;
        keys.push_back(mi);
        return;
    }
    else if (basestr == "mousebutton")
    {
        MouseButtonInputStyle mi;
        mi.inputName = name;
        mi.key = num;
        mi.reversed = invert;
        mbuttons.push_back(mi);
        return;
    }
    else
    {
        Log << "Invalid input type at \"" << original << "\"" << Trace(CHash("Warning"));
        return;
    }
        
}

void KeyConfig::write(const std::string& fn)
{
    //Log << "KeyConfig::write() unimplemented" << Trace(CHash("Warning"));

    auto file = GetFileWriter(fn);
    
    if (file)
    {
        auto filestreambuf = std::make_unique<FileWriterStreambuf>(std::move(file));
        std::ostream os(filestreambuf.get());
        
        for (auto p : keys)
        {
            os << "Key ";
            
            auto it = keyAliasesAssoc.find(p.key);
            if (it == keyAliasesAssoc.end())
                os << p.key;
            else
                os << it->second;
            
            if (p.reversed)
                os << " Inverted";
            
            os << " = " << p.inputName;
            os << std::endl;
        }
        for (auto p : mbuttons)
        {
            os << "MouseButton ";
            os << p.key;
            if (p.reversed) os << " Inverted";
            os << " = " << p.inputName;
            os << std::endl;
        }
        
        for (auto p : jbuttons)
        {
            os << "Joy ";
            os << p.joy << " Button ";
            os << p.button;
            if (p.reversed) os << " Inverted";
            os << " = " << p.inputName;
            os << std::endl;
        }

        for (auto p : axes)
        {
            
            os << "Joy ";
            os << p.joy << " Axis ";
            os << p.axis;
            if (p.reversed) os << " Inverted";
            os << " = " << p.inputName;
            os << std::endl;
        }

    }


}
void KeyConfig::load(const std::string& fn)
{
    keys.clear();
    jbuttons.clear();
    mbuttons.clear();
    axes.clear();

    
    auto file = GetFileStream(fn);

    if (!file)
        return;
    
    while (!file->isEOF())
    {
        auto buffer = file->readLine();
        
        std::string spl = StringUtil::SplitFirst(buffer, '=');
        
        StringUtil::Trim(spl);
        if (spl.size() == 0)
            continue;
        
        StringUtil::Trim(buffer);
        
        if (buffer.size() <= 1)
            continue;

        addValue(buffer, spl);
    }

}


void KeyConfig::setValues(std::vector<JoyAxisInputStyle>& a, std::vector<JoyButtonInputStyle>& b, std::vector<KeyInputStyle> & c, std::vector<MouseButtonInputStyle>& d)
{
    axes = a;
    jbuttons = b;
    keys = c;
    mbuttons = d;
}
