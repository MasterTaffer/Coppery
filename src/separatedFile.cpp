#include "separatedFile.hpp"
#include "fileOperations.hpp"
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include <gbvfs.hpp>

double SeparatedRow::getDouble(int index, double def)
{
    if (elements.size() > (unsigned int) index && index >= 0)
    {
        std::string& s = elements.at(index);
        std::stringstream ss;
        ss << s;
        double k;
        ss >> k;
        return k;
    }
    return def;
}

int SeparatedRow::getInt(int index, int def)
{
    if (elements.size() > (unsigned int) index && index >= 0)
    {
        std::string& s = elements.at(index);
        std::stringstream ss;
        ss << s;
        int k;
        ss >> k;
        return k;
    }
    return def;
}

std::string SeparatedRow::getString(int index, std::string def)
{
    if (elements.size() > (unsigned int) index && index >= 0)
    {
        std::string& s = elements.at(index);
        std::stringstream ss;
        ss << s;
        std::string k;
        ss >> k;
        return k;

    }
    return def;

}

SeparatedFile::SeparatedFile(const std::string& fn, char c)
{
    auto file = GetFileStream(fn);

    if (!file)
        return;

    while (!file->isEOF())
    {
        auto buffer = file->readLine();

        if (buffer.size() == 0)
            continue;
        if (buffer[0] == '#')
            continue;
        SeparatedRow sr;

        std::stringstream ss;
        ss << buffer;
        while (std::getline(ss,buffer,c))
        {
            sr.elements.push_back(buffer);
        }

        rows.push_back(sr);
    }
}


SeparatedFile SeparatedFile::BatchLoad(const std::string& prefix, const std::string& ending, char c)
{
    SeparatedFile sf;

    std::vector<std::string> ff;
    GetFilesInDirectoryRecursive(prefix, ending,ff);

    for (auto& fn : ff)
    {

        auto file = GetFileStream(fn);

        if (!file)
            continue;
        
        while (!file->isEOF())
        {
            auto buffer = file->readLine();

            if (buffer.size() == 0)
                continue;
            if (buffer[0] == '#')
                continue;
            SeparatedRow sr;

            std::stringstream ss;
            ss << buffer;
            while (std::getline(ss,buffer,c))
            {
                sr.elements.push_back(buffer);
            }

            sf.rows.push_back(sr);
        }
    }
    return sf;
}
