
#include "fileHandleStream.hpp"
#include <gbvfs.hpp>
#ifndef COPPERY_HEADLESS

FileHandleStream::FileHandleStream(std::unique_ptr<FileHandle> h) : handle(std::move(h))
{
    
}

FileHandleStream::~FileHandleStream()
{

}

sf::Int64 FileHandleStream::read(void* data, sf::Int64 size)
{
    if (!handle)
        return -1;

    return handle->read(data, size);
}

sf::Int64 FileHandleStream::seek(sf::Int64 position)
{
    if (!handle)
        return -1;

    handle->seek(position);
    return handle->tell();
}

sf::Int64 FileHandleStream::tell()
{
    if (!handle)
        return -1;

    return handle->tell();
}

sf::Int64 FileHandleStream::getSize()
{
    if (!handle)
        return -1;
    return handle->getSize();
}
#endif
