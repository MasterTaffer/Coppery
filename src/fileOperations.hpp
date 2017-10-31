 #pragma once
#include <string>
#include <vector>
#include <fstream>
#include <memory>

#include "gbvfs_file.hpp"

/*! \file fileOperations.cpp
    \brief Tools and routines to interact with the (virtual) file system
*/


extern std::string AssetsFolder;
extern std::string ControlScriptsFolder;
extern std::string ScriptsFolder;
extern std::string ScriptsCompiledFolder;

class ArgumentParser;


//! Initialize the virtual file system
void VFS_Init(ArgumentParser&);
//! Shut down the virtual file system
void VFS_Deinit();

/*! \brief Get files in directory recursively
 * 
 * \param dirstr the directory to start the search in
 * \param fileEnding ending of the files to include in results
 * \param ret output array of file names
 */
void GetFilesInDirectoryRecursive(const std::string& dirstr, const std::string& fileEnding, std::vector<std::string> & ret);

//! Query if \p filename exists within virtual file system
bool FileExists(const std::string& filename);

typedef GBVFS::File FileHandle;
typedef GBVFS::FileWriter FileWriterHandle;

//! Canonicalize the input path
std::string CanonicalizePath(const std::string& filename);

//! Get a GBVFS::File handle to \p filename
std::unique_ptr<FileHandle> GetFileStream(const std::string& filename);

//! Get a GBVFS::FileWriter handle to \p filename
std::unique_ptr<FileWriterHandle> GetFileWriter(const std::string& filename);

//! Load \p filename to memory. User must delete the returned buffer afterwards.
char* GetFileContentsCopy(const std::string& filename, size_t* size = nullptr);

//! Load \p filename to memory. User shouldn't delete or modify the returned buffer.
const char* GetFileContentsMaintainedCopy(const std::string& filename, size_t* size = nullptr);

//! Convert a VFS filename to real filesystem name (if possible)
std::string GetFileRealName(const std::string& );

//! Return the filename extension. Returns the part after the final dot.
std::string GetFileExtension(const std::string& );

//! Return filename without extension(s)
std::string GetFileNameWithoutExtension(const std::string& );

//! Return path without extensions(s)
std::string GetPathWithoutExtension(const std::string& );

/*! \brief Return relative path \p path applied to absolute path \p relativeTo
 *
 * The \p relativeTo should be a reference to a filename in the folder,
 * not the folder itself. 
 */

std::string GetPathRelativeTo(const std::string& path, const std::string& relativeTo);

enum {
    WriteAccessPerUser = 1,
    WriteAccessGlobal = 2
};

std::unique_ptr<FileHandle> GetUserFileStream(const std::string& filename);

std::ofstream OpenWriteAccessFile(int writeAccessType, std::string filename, std::ios_base::openmode mode);
std::ofstream OpenWriteAccessFile(std::string rl, std::ios_base::openmode mode);
