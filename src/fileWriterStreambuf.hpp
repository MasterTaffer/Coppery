#include "gbvfs_file.hpp"
#include <streambuf>
#include <memory>


//! std::streambuf implementation using GBVFS FileWriter
class FileWriterStreambuf : public std::streambuf
{
    std::unique_ptr<GBVFS::FileWriter> handle;
public:
    
    //! Override from std::streambuf
    std::streamsize xsputn(const char_type* s, std::streamsize n);
    
    //! Override from std::streambuf
    int_type overflow(int_type c);
    
    //! Constructor from GBVFS FileWriter handle
    FileWriterStreambuf(std::unique_ptr<GBVFS::FileWriter> handle);
    
    //! Destructor
    ~FileWriterStreambuf();
};
