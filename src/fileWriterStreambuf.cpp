#include "fileWriterStreambuf.hpp"


std::streamsize FileWriterStreambuf::xsputn(const char_type* s, std::streamsize n)
{
    if (!handle)
        return 0;
    return handle->write(static_cast<const void*>(s), n);
}    

int FileWriterStreambuf::overflow(int c)
{
    if (!handle)
        return EOF;
    char c2 = c;
    handle->write(static_cast<const void*>(&c2), 1);
    return c;
}
    

FileWriterStreambuf::FileWriterStreambuf(std::unique_ptr<GBVFS::FileWriter> handle)
: handle(std::move(handle))
{
}
    

FileWriterStreambuf::~FileWriterStreambuf()
{
}

