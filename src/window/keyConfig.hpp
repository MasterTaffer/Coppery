#pragma once
#include "keys.hpp"

#include <vector>
class KeyConfig
{
    std::unordered_map<std::string, int> keyAliases;
    std::unordered_map<int, std::string> keyAliasesAssoc;
    std::string file;
    std::vector<JoyAxisInputStyle> axes;
    std::vector<JoyButtonInputStyle> jbuttons;
    std::vector<KeyInputStyle> keys;
    std::vector<MouseButtonInputStyle> mbuttons;
    void addValue(const std::string&, const std::string&);
public:
    KeyConfig();
    void load(const std::string&);
    std::vector<JoyAxisInputStyle>& getAxes() {return axes;};
    std::vector<JoyButtonInputStyle>& getJoyButtons() {return jbuttons;};
    std::vector<KeyInputStyle>& getKeys() {return keys;};
    std::vector<MouseButtonInputStyle>& getMouseButtons() {return mbuttons;};

    void write(const std::string&);
    void setValues(
    std::vector<JoyAxisInputStyle>&,
    std::vector<JoyButtonInputStyle>&,
    std::vector<KeyInputStyle>&,
    std::vector<MouseButtonInputStyle>&);

};
