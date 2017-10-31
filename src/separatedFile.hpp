#pragma once
#include <string>
#include <vector>


class SeparatedRow
{
    std::vector<std::string> elements;
public:
    double getDouble(int index, double def = 0.0f);
    int getInt(int index, int def = 0);
    std::string getString(int index, std::string def = "");
    
    friend class SeparatedFile;
};
class SeparatedFile
{
public:
    SeparatedFile() = default;
    SeparatedFile(const std::string& file,char separator = ':');
    std::vector<SeparatedRow> rows;
    static SeparatedFile BatchLoad(const std::string& prefix, const std::string& ending, char separator = ':');
};
