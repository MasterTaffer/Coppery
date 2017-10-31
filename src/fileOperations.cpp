#include "fileOperations.hpp"
#include "log.hpp"
#include "argumentParser.hpp"

#include <unordered_map>
#include <cstring>

#define GBVFS_IMPLEMENTATION
#include <gbvfs.hpp>
#include <tinydir.h>

//Because tinydir sucks
#define GFT_TYPE_FILE 1
#define GFT_TYPE_DIR 2


static int GetFileType(const char* path);

#ifdef _MSC_VER

#include <windows.h>
#include <tchar.h>

static int GetFileType(const char* path)
{
	auto ret = GetFileAttributes(path);
	if (ret == INVALID_FILE_ATTRIBUTES)
		return 0;
	if ((ret & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return GFT_TYPE_DIR;
	return GFT_TYPE_FILE;
}

#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static int GetFileType(const char* path)
{
	struct stat fstat;
	if (stat(path, &fstat) == -1)
	{
		return 0;
	}
	if (S_ISDIR(fstat.st_mode))
	{
		return GFT_TYPE_DIR;
	}
	if (S_ISREG(fstat.st_mode))
	{
		return GFT_TYPE_FILE;
	}
	return 0;
}

#endif



//! GBVFS::File implementation for C standard file operations
class CFile : public GBVFS::File
{
    FILE* file;
    long size;
public:
    bool isOpen;

    void seek(unsigned long offs) override
    {
        if (file && isOpen)
            fseek(file, offs, SEEK_SET);

    }
    unsigned long tell() override
    {
        if (file && isOpen)
            return ftell(file);
        return 0;
    }

    size_t read(void* buf, size_t bytes) override
    {
        if (file && isOpen)
            return fread(buf, 1, bytes, file);
        return 0;
    }
    long getSize() override
    {
        return size;
    }
    bool isEOF() override
    {
        if (file && isOpen)
            return feof(file);
        return true;
    }
    void close() override
    {
        if (!isOpen)
            return;
        fclose(file);
        isOpen = false;
    }

    CFile(const char* fpath)
    {
        size = 0;
        file = fopen(fpath, "rb");
        if (file)
        {
            isOpen = true;
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            fseek(file, 0, SEEK_SET);
        }
        else
            isOpen = false;
    }

    virtual ~CFile()
    {
        if (isOpen)
            close();
    }
};


//! GBVFS::FileWriter implementation for C standard file operations
class CFileWriter : public GBVFS::FileWriter
{
    FILE* file;
public:
    bool isOpen;

    size_t write(const void* buf, size_t bytes) override
    {
        if (file && isOpen)
            return fwrite(buf, 1, bytes, file);
        return 0;
    }
    void close() override
    {
        if (!isOpen)
            return;
        fclose(file);
        isOpen = false;
    }

    CFileWriter(const char* fpath)
    {
        file = fopen(fpath, "wb");
        if (file)
        {
            isOpen = true;
        }
        else
            isOpen = false;
    }

    virtual ~CFileWriter()
    {
        if (isOpen)
            close();
    }
};


//! GBVFS::Loader implementation using tinydir
class TinyDirLoader : public GBVFS::Loader
{
public:
    virtual void iterateEntries(const GBVFS::PathView& pv, FileIteratorCallback fic)
    {
        tinydir_dir dir;
        int res = 0;
        if (pv.getComponentCount() == 0)
            res = tinydir_open(&dir, ".");
        else
            res = tinydir_open(&dir, pv.getCString());
        
        if (res != 0) //Error
            return;

        while (dir.has_next)
        {
            tinydir_file file;
            tinydir_readfile(&dir, &file);

            if (file.name[0] != '.')
            {
                GBVFS::FileEntry f;
                f.exists = true;

                if (file.is_dir)
                {
                    f.directory = true;
                }
                fic(file.name, f);
            }
            tinydir_next(&dir);
        }
        tinydir_close(&dir);

    }

    virtual GBVFS::FileEntry getFileEntry(const GBVFS::PathView& path)
    {
        GBVFS::FileEntry f;
        tinydir_file file;
        const char* t = ".";
        if (path.getComponentCount() > 0)
            t = path.getCString();
		auto val = GetFileType(t);
		if (val == 0)
			return f;
		f.exists = true;
		if (val == GFT_TYPE_DIR)
			f.directory = true;
		
        return f;
    }

    virtual std::unique_ptr<GBVFS::File> openFile(const GBVFS::PathView& path)
    {
        auto f = std::make_unique<CFile>(path.getCString());
        if (f->isOpen)
            return std::move(f);
        return nullptr;
    }
    
    
    virtual std::unique_ptr<GBVFS::FileWriter> openFileWrite(const GBVFS::PathView& path)
    {
        auto f = std::make_unique<CFileWriter>(path.getCString());
        if (f->isOpen)
            return std::move(f);
        return nullptr;
    }
};


//Used for caching, etc.
//GBVFS functions automatically perform canonicalization via Path 
std::string CanonicalizePath(const std::string& filename)
{
    GBVFS::Path path;
    
    path.fromCStr(filename.c_str(), true);
    return path.fullPath;
}

static std::unordered_map<std::string, std::pair<size_t, std::unique_ptr<char[]>>> MaintainedFiles;
static std::unique_ptr<GBVFS::VFS> VFSInstance;

void VFS_Init(ArgumentParser& args)
{
    std::string base_vfs = "./";
    
    args.getArgumentFullString("", "--working-dir", base_vfs);
    base_vfs = base_vfs + "/";

    std::string wdir = base_vfs + "data";
    args.getArgumentFullString("", "--data", wdir);

    VFSInstance = std::make_unique<GBVFS::VFS>();

    auto idx = VFSInstance->registerLoader(std::make_unique<TinyDirLoader>());

    auto mounthelp = [&](const std::string& from, const std::string& to, int flags = 0)
    {
        GBVFS::Path p;
        p.fromCStr(from.c_str(), true);

        int err = VFSInstance->mount(idx, p.c_str(), to.c_str(), flags);
        
        Log << "VFS mount \"" << p.c_str() << "\" -> \"" << to << "\": " << VFSInstance->getMountReturnCodeInformation(err);
        
        if (err != 0)
            Log << Trace(CHash("Warning"));
        else
            Log << Trace(CHash("General"));
    };

    std::string adir = wdir + "/" + "assets";
    std::string cdir = wdir + "/" + "control";
    std::string asdir = wdir + "/" + "angelscript";
    std::string compdir = wdir + "/" + "compiled";
        
    
    args.getArgumentFullString("", "--dir-control", cdir);
    args.getArgumentFullString("", "--dir-angelscript", asdir);
    args.getArgumentFullString("", "--dir-compiled", compdir);
    
    mounthelp(adir, AssetsFolder);
    mounthelp(cdir, ControlScriptsFolder);
    mounthelp(asdir, ScriptsFolder);
    mounthelp(compdir, ScriptsCompiledFolder, GBVFS::VFS::WRITEACCESS);
    
    
    std::string staticdir = base_vfs + "static";
    args.getArgumentFullString("", "--dir-static", staticdir);
    
    mounthelp(staticdir, "static");
    
    std::string base_usr = base_vfs;
    args.getArgumentFullString("", "--dir-user", base_usr);
    base_usr = base_usr + "/";
    
    std::string cfgdir = base_usr + "config";
    std::string userdatadir = base_usr + "userdata";
    
    args.getArgumentFullString("", "--dir-config", cfgdir);
    args.getArgumentFullString("", "--dir-user-data", userdatadir);
    
    mounthelp(cfgdir, "user/config", GBVFS::VFS::WRITEACCESS);
    mounthelp(userdatadir, "user/data", GBVFS::VFS::WRITEACCESS);
}

void VFS_Deinit()
{
    VFSInstance = nullptr;
    MaintainedFiles.clear();
}


std::string AssetsFolder = "assets";
std::string ControlScriptsFolder = "control";
std::string ScriptsFolder = "angelscript";
std::string ScriptsCompiledFolder = "compiled";

void GetFilesInDirectoryRecursive(const std::string& dirstr, const std::string& fileEnding, std::vector<std::string> & ret)
{
    GBVFS::Path dirpath(dirstr);

    auto dir = VFSInstance->getDirectory(dirpath.fullPath.c_str());

    for (auto& p : dir)
    {
        if (p.second.directory)
        {
            GetFilesInDirectoryRecursive(dirpath.fullPath + "/" + p.first, fileEnding, ret);
        }
        else
        {
            auto& name = p.first;
            int l = name.length();
            int fel = fileEnding.length();
            if (l >= fel)
            {
                bool match = true;
                for (int i = 0; i < fel; i++)
                {
                    if (name[i+l-fel] != fileEnding[i])
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                {
                    std::string ak = dirpath.fullPath+"/"+name;
                    ret.push_back(ak);
                }
            }
        }
    }
}


std::unique_ptr<FileHandle> GetFileStream(const std::string& filename)
{
    return VFSInstance->openFile(filename.c_str());
}

std::unique_ptr<FileWriterHandle> GetFileWriter(const std::string& filename)
{
    return VFSInstance->openFileWrite(filename.c_str());
}


std::string GetFileRealName(const std::string& f)
{
    return VFSInstance->getRealPath(f.c_str());
}

const char* GetFileContentsMaintainedCopy(const std::string& filename_nonc, size_t* size)
{
    GBVFS::Path path(filename_nonc);
    std::string filename = path.fullPath;

    auto it = MaintainedFiles.find(filename);
    if (it != MaintainedFiles.end())
    {
        if (size)
            *size = it->second.first;
        return it->second.second.get();
    }
    size_t sz;
    char* r = GetFileContentsCopy(filename, &sz);

    if (size)
        *size = sz;
    if (r)
    {
        MaintainedFiles[filename] = {sz, std::unique_ptr<char[]>(r)};
        return r;
    }
    return nullptr;
}

//Gets a copy of the file contents
//User must delete the data afterwards
char* GetFileContentsCopy(const std::string& filename, size_t* size)
{
    auto f = VFSInstance->openFile(filename.c_str());
    if (f)
    {
        size_t sz = f->getSize();
        if (size)
            *size = sz;

        if (sz == 0)
        {
            return nullptr;
        }

        //allocate a copy
        char* ret = new char[sz+1];
        if (!ret)
        {
            return nullptr;
        }

        //read the copy
        sz = f->read((void*)ret, sz);
        
        f->close();

        //Set terminating character
        ret[sz] = 0;

        return ret;
    }
   
    return nullptr;
}

bool FileExists(const std::string& filename)
{
    GBVFS::FileEntry fe = VFSInstance->getFileEntry(filename.c_str());
    if (fe.exists && !fe.directory)
        return true;
    return false;
}

std::string GetPathRelativeTo(const std::string& path, const std::string& relativeTo)
{
    std::string np = path;
    std::size_t found = relativeTo.find_last_of('/');
    if (found != relativeTo.npos)
        np = relativeTo.substr(0, found + 1) + np;
    return np;
}

std::string GetFileNameWithoutExtension(const std::string& fn_o)
{
    std::string fn = fn_o;
    std::size_t found = fn.find_last_of('/');
    if (found != fn.npos)
        fn = fn.substr(found+1);

    found = fn.find_last_of(".");
    if (found != fn.npos)
        fn = fn.substr(0,found);
    return fn;
}

std::string GetFileExtension(const std::string& fn)
{
    std::string ext = "";

    std::size_t found = fn.find_last_of(".");
    if (found != fn.npos)
        ext = fn.substr(found + 1);
    return ext;
}

std::string GetPathWithoutExtension(const std::string& fn_o)
{
    std::string fn = fn_o;
    std::size_t found = fn.find_last_of(".");
    if (found != fn.npos)
        fn = fn.substr(0,found);
    return fn;
}

std::unique_ptr<FileHandle> GetUserFileStream(const std::string& filename)
{
    auto f = std::make_unique<CFile>(filename.c_str());
    if (f->isOpen)
        return std::move(f);
    return nullptr;
}

std::ofstream OpenWriteAccessFile(int writeAccessType, std::string filename, std::ios_base::openmode mode)
{
    if (writeAccessType == WriteAccessGlobal)
    {
        filename = "./" + filename;
        return std::ofstream(filename.c_str(), mode | std::ofstream::out);
    }
    else
    {
        filename = "./userdata/" + filename;
        return std::ofstream(filename.c_str(), mode | std::ofstream::out);
    }
    return std::ofstream();
}

std::ofstream OpenWriteAccessFile(std::string rl, std::ios_base::openmode mode)
{
    auto p = rl.find(':');
    if (p == rl.npos)
    {
        Log << "Invalid write access specifier on resource locator '" << rl << "'" << Trace(CHash("Warning"));
        return std::ofstream();
    }
    auto s = rl.substr(0, p);
    rl = rl.substr(p + 1);

    if (s == "user")
        return OpenWriteAccessFile(WriteAccessPerUser, rl, mode);

    if (s == "global")
        return OpenWriteAccessFile(WriteAccessGlobal, rl, mode);

    Log << "Invalid write access specifier on resource locator '" << rl << "'" << Trace(CHash("Warning"));
    return std::ofstream();
}

