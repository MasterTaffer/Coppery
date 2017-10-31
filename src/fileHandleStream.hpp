

#pragma once

class FileHandleStream;

#ifndef COPPERY_HEADLESS
#include <SFML/System.hpp>

#include "fileOperations.hpp"

//! GBVFS File wrapper for SFML sf::InputStream  
class FileHandleStream : public sf::InputStream
{
    std::unique_ptr<FileHandle> handle;
public:

    //! Constructor from GBVFS File handle
    FileHandleStream(std::unique_ptr<FileHandle> handle);
    
    //! Destructor
    ~FileHandleStream();

    //! Read \p size bytes to \p data
    virtual sf::Int64 read(void* data, sf::Int64 size);

    //! Seek into position \p position
    virtual sf::Int64 seek(sf::Int64 position);

    //! Return current position in the stream
    virtual sf::Int64 tell();

    //! Get the total size of stream
    virtual sf::Int64 getSize();

};
#endif

