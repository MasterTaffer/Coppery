#include "gbvfs_file.hpp"
#include <streambuf>
#include <memory>


//! std::streambuf implementation using GBVFS File
class FileStreambuf : public std::streambuf
{
    std::unique_ptr<GBVFS::File> handle;
    char buffer[128];
    bool eof = false;
public:
    //! Override from std::streambuf
    int underflow();
    
    //! Constructor from GBVFS File handle
    FileStreambuf(std::unique_ptr<GBVFS::File> handle);
    
    //! Destructor
    ~FileStreambuf();
};

