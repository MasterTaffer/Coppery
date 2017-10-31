#include "fileStreambuf.hpp"



    
FileStreambuf::FileStreambuf(std::unique_ptr<GBVFS::File> handle)
    : handle(std::move(handle))
{

}

FileStreambuf::~FileStreambuf()
{
}

int FileStreambuf::underflow()
{
    if (gptr() == egptr())
    {
        if (!eof)
        {
            auto v = handle->read((char*)buffer, sizeof(buffer));
            if (v < sizeof(buffer))
                eof = true;
            setg(buffer, buffer, buffer + v);
        }
    }
    return this->gptr() == this->egptr()
         ? std::char_traits<char>::eof()
         : std::char_traits<char>::to_int_type(*this->gptr());
    
}