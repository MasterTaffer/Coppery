#define DATASER_SERIALIZATION_IMPLEMENTATION
#include "dataser.hpp"

#include "dataserFile.hpp"
#include "fileOperations.hpp"
#include "log.hpp"

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

#include "hash.hpp"

#include "fileWriterStreambuf.hpp"
#include "fileStreambuf.hpp"

namespace Dataser
{
    Table* ReadXMLFromFile(const std::string& file)
    {
        size_t t;
        char* c = GetFileContentsCopy(file,&t);
        if (!c)
            return nullptr;
        auto p = ReadXMLFromMemory(c,file);
        delete[] c;

        if (p.table)
        {
            return p.table;
        }
        else
        {
            Log << "Failed to read serialized data from '" << GetFileRealName(file) << "': " << p.errorMessage << Trace(CHash("Warning"));
        }
        return nullptr;
    }


    Table* ReadTOMLFromFile(const std::string& file)
    {
        auto f = GetFileStream(file);
        if (!f)
            return nullptr;
        TableOrError p;

        {
            auto m = std::make_unique<FileStreambuf>(std::move(f));
            std::istream istream(m.get());
            p = ReadTOMLFromStream(istream);
        }
            
        if (p.table)
        {
            return p.table;
        }
        else
        {
            Log << "Failed to read serialized data from '" << GetFileRealName(file) << "': " << p.errorMessage << Trace(CHash("Warning"));

        }
        return nullptr;
    }

    bool WriteXMLToFile(Table* t, const std::string& file)
    {
        auto p = GetFileWriter(file);
        if (p)
        {
            auto m = std::make_unique<FileWriterStreambuf>(std::move(p));
            std::ostream os(m.get());
            WriteXMLToStream(t, os);
            return true;
        }
        else
            Log << "Failed to write serialized data to '" << file << "'" << Trace(CHash("Warning"));
        return false;
    }


    Table* ReadFromFile(const std::string& file)
    {
        std::string ext = GetFileExtension(file);
        if (ext == "xml")
        {
            return ReadXMLFromFile(file);
        }
        if (ext == "toml")
        {
            return ReadTOMLFromFile(file);
        }
        Log << "Failed to read serialized data from '" << GetFileRealName(file) << "': unknown file type" << Trace(CHash("Warning"));

        return nullptr;
    }

}
